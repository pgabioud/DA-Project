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
            send("hum", 255, p->id);
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

int UDP::send(const char * msg, size_t size, int p_id) {
    auto *p = m_procs[p_id - 1];

    string seqNumb = to_string(seqNum);
    string newMsg = to_string(curr_proc + 1) + " " + seqNumb;

    cout << "Sending to socket : [" << p->socket << "] ... message : "<< newMsg <<   endl;
    int er = sendto(p->socket, newMsg.c_str(), size, 0, p->addrinfo->ai_addr, p->addrinfo->ai_addrlen);
    if(er < 0) {
        cerr << "Error sending message : " << newMsg << endl;
    }

    return er;
}

Message UDP::rcv(char *msg, size_t size) {
    int sockfd = m_procs[curr_proc]->socket;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    char host[NI_MAXHOST], service[NI_MAXSERV];

    cout << "Receiving on socket : [" << sockfd <<"]" <<  endl;
    int er = ::recvfrom(sockfd, msg, size, 0, (struct sockaddr *) &peer_addr, &peer_addr_len);

    int s = getnameinfo((struct sockaddr *) &peer_addr, peer_addr_len, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
    if (s == 0) {
        Message message = new Message();
        return message;
    } else {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
    }

    if(er < 0) {
        cerr << "Error receiving" << endl;
        return ???;
    }
}


//Stubborn Links Modules
StubbornLinks::StubbornLinks(vector<process *> &procs, int id)
:UDP(procs, id)
{}

int StubbornLinks::send(const char *msg, size_t size, int p_id) {
    clock_t start;
    start = clock();

    while(((clock() - start) / (double) CLOCKS_PER_SEC) < timeout) {
        UDP::send(msg, size, p_id);
    }
}

Message StubbornLinks::rcv(char *msg, size_t size) {
    UDP::rcv(msg, size);
}


//Perfect Links Module
PerfectLinks::PerfectLinks(vector<process *> &procs, int id)
:StubbornLinks(procs, id)
{}

int PerfectLinks::send(const char *msg, size_t size, int p_id) {
    StubbornLinks::send(msg, size, p_id);
}

Message PerfectLinks::rcv(char *msg, size_t size) {
    Message message = StubbornLinks::rcv(msg, size);
    if(!(find(delivered.begin(), delivered.end(), message) != delivered.end())) {
        delivered.push_back(message);
        return message;
    }
    Message alreadyDeliveredMessage = Message(-1, " ");
    return alreadyDeliveredMessage;
}
