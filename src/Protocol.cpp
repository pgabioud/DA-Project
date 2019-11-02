//
// Created by rudra on 01.10.19.
//
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <sstream>
#include <netdb.h>
#include <cstring>
#include <algorithm>
#include "Protocol.h"

vector<vector<string>> logBuffer;

void Protocol::init_socket(process* proc) {
    int m_port = proc->port;
    string m_addr = proc->ip;
    struct addrinfo * m_addrinfo = proc->addrinfo;
    char decimal_port[16];
    snprintf(decimal_port, sizeof(decimal_port), "%d", m_port);
    decimal_port[sizeof(decimal_port) / sizeof(decimal_port[0]) - 1] = '\0';
    struct addrinfo ai;
    memset(&ai, 0, sizeof(ai));
    ai.ai_family = AF_UNSPEC;
    ai.ai_socktype = SOCK_DGRAM;
    ai.ai_protocol = IPPROTO_UDP;
    int r(getaddrinfo(m_addr.c_str(), decimal_port, &ai, &m_addrinfo));
    if(r != 0 || m_addrinfo == NULL)
    {
        throw runtime_error(("invalid address or port: \"" + m_addr + ":" + decimal_port + "\"").c_str());
    }

    //create socket
    int m_socket = socket(m_addrinfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
    if(m_socket == -1)
    {
        freeaddrinfo(m_addrinfo);
        throw runtime_error(("could not create socket for: \"" + m_addr + ":" + decimal_port + "\"").c_str());
    }

    // bind socket
    if(proc->id == curr_proc + 1) {
        r = bind(m_socket, m_addrinfo->ai_addr, m_addrinfo->ai_addrlen);
        if(r != 0)
        {
            //freeaddrinfo(m_addrinfo);
            cerr << "Could not bind socket to " << m_addr << " on port :" << m_port << endl;
        }
    }


    proc->socket = m_socket;
    proc->addrinfo = m_addrinfo;
}

Protocol::Protocol(vector<process*> &processes, int curr_id)
:m_procs(processes), curr_proc(curr_id - 1)
{
    for(auto & p: m_procs) {
        init_socket(p);
        cout << *p << endl;
    }

    //init variables
    seqNum = 1;
    stringstream ss;
    ss << "da_proc_" << curr_id << ".out" <<  endl;
    log = ss.str();


}

UDP::UDP(vector<process*> &procs, int id)
:Protocol(procs, id)
{}

int Protocol::broadcast() {
    for(auto p : m_procs) {
        if(p->id!= curr_proc + 1) {
            stringstream ss;
            cout << "Sending : " << ss.str();
            Message message = Message(curr_proc, p->id, to_string(seqNum).c_str(), 255, false);
            send(&message);
            //send("hum", 255, p->id);
            cout << "Message sent to " << p->id <<  endl;
            ss.clear();
        }
    }
    string seqNumb = to_string(seqNum);
    vector<string> newLog = {"b", seqNumb};
    logBuffer.push_back(newLog);
    if(logBuffer.size() <= sizeBuffer) {
        writeLogs(log, &logBuffer);
        logBuffer.clear();
    }

    seqNum++;
}

int UDP::send(Message *message) {
    auto *p = m_procs[message->sid - 1];

    string seqNumString = to_string(seqNum);

    cout << "Sending to socket : [" << p->socket << "] ... message : "<< seqNumString <<   endl;
    int er = sendto(p->socket, seqNumString.c_str(), message->size, 0, p->addrinfo->ai_addr, p->addrinfo->ai_addrlen);
    if(er < 0) {
        cerr << "Error sending message : " << seqNumString << endl;
    }
    return er;
}

Message* UDP::rcv(Message *message) {
    int sockfd = m_procs[curr_proc]->socket;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    char host[NI_MAXHOST], service[NI_MAXSERV];
    char *msg;

    cout << "Receiving on socket : [" << sockfd <<"]" <<  endl;
    int er = ::recvfrom(sockfd, &message->seqNum, message->size, 0, (struct sockaddr *) &peer_addr, &peer_addr_len);

    int s = getnameinfo((struct sockaddr *) &peer_addr, peer_addr_len, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
    if (s == 0) {
        int idSource = stringToInt(service)%11000;
        message->sid = idSource;
        message->did = curr_proc;
        message->ack = false;
        return message;
    } else {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
    }

    if(er < 0) {
        cerr << "Error receiving" << endl;
        message->sid = -1;
        message->did = -1;
        message->seqNum = " ";
        message->size = 0;
        message->ack = false;
        return message;
    }
}


//Stubborn Links Modules
StubbornLinks::StubbornLinks(vector<process *> &procs, int id)
:UDP(procs, id)
{}

int StubbornLinks::send(Message *message) {
    /*
    clock_t start;
    start = clock();

    while(((clock() - start) / (double) CLOCKS_PER_SEC) < timeout) {
        UDP::send(msg, size, p_id);
    }
    */
    while(find(curr_sending.begin(), curr_sending.end(), message) != curr_sending.end()) {
        UDP::send(message);
    }
}

Message* StubbornLinks::rcv(Message *message) {
    if (!message->ack) {
        UDP::rcv(message);
        Message ackMessage = Message(message->did, message->sid, message->seqNum, message->size, true);
    } else {
        vector<Message*>::iterator it;
        for (it = curr_sending.begin(); it != curr_sending.end(); ++it) {
            if((*it) == message) {
                curr_sending.erase(it);
            }
        }
    }
}


//Perfect Links Module
PerfectLinks::PerfectLinks(vector<process *> &procs, int id)
:StubbornLinks(procs, id)
{}

int PerfectLinks::send(Message *message) {
    StubbornLinks::send(message);
}

Message* PerfectLinks::rcv(Message *message) {
    vector<Message*>::iterator it;
    for (it = delivered.begin(); it != delivered.end(); ++it) {
        if((*it) == message) {
            delivered.push_back(message);
            StubbornLinks::rcv(message);
            return message;
        }
    }
    Message alreadyDeliveredMessage = Message(-1, -1, " ", 0, false);
    Message* alreadyDeliveredMessagePointer = &alreadyDeliveredMessage;
    return alreadyDeliveredMessagePointer;
}
