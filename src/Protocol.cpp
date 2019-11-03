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
#include <fstream>
#include "Protocol.h"

vector<vector<string>> logBuffer;

#define ACK "ack"

void Protocol::init_socket(process* proc) {
    int port = proc->port;
    string m_addr = proc->ip;

    struct sockaddr_in* addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    int fd;

    if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket");
        exit(1);
    }

    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(m_addr.c_str());
    addr->sin_port = htons(port);

    if(curr_proc + 1== proc->id) {
        if (bind(fd, (struct sockaddr *) addr, sizeof(*addr)) < 0) {
            perror("bind");
            exit(1);

        }
    }

    proc->addrinfo = addr;
    proc->socket = fd;
}

Protocol::Protocol(vector<process*> &processes, int curr_id)
:m_procs(processes), curr_proc(curr_id - 1)
{

    for(auto & p: m_procs) {
            init_socket(p);
    }
    cout << "Sockets inited" << endl;
    for(auto p:m_procs) {
        cout << *p << endl;
    }

    //init variables
    seqNum = 1;
    stringstream ss;
    ss << "da_proc_" << curr_id << ".out" <<  endl;
    log = ss.str();
    ofstream ofs;
    ofs.open(log, std::ofstream::out | std::ofstream::trunc);
    ofs.close();
}

UDP::UDP(vector<process*> &procs, int id)
:Protocol(procs, id)
{}

int Protocol::broadcast() {
    //string m = to_string(curr_proc + 1) + " " + to_string(seqNum);
    string m = "ack " + to_string(seqNum);
    for(auto p : m_procs) {
        if(p->id!= curr_proc + 1) {
            Message message = Message(curr_proc, p->id - 1, m, m.size(), false);
            send(&message);
        }
    }
    string seqNumb = to_string(seqNum);
    vector<string> newLog = {"b", seqNumb};
    logBuffer.push_back(newLog);
    if(logBuffer.size() <= sizeBuffer) {
        writeLogs(log, &logBuffer);
        logBuffer.clear();
    }

    cout << "Broadcast done" << endl;
    seqNum++;
}

int UDP::send(Message *message) {
    auto *p = m_procs[message->did];

    cout << "Sending to socket : [" << p->socket << "] message : ["<< message->payload << "]" <<  endl;
    int er = sendto(p->socket, message->payload.c_str(), message->payload.size(), 0, (const sockaddr*)(p->addrinfo), sizeof(*p->addrinfo));
    if(er < 0) {
        cerr << "Error sending message : " << message << endl;
    }
    return er;
}

Message* UDP::rcv(Message *upper_m) {
    int sockfd = m_procs[curr_proc]->socket;
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len;
    peer_addr_len = sizeof(peer_addr);
    char host[NI_MAXHOST], service[NI_MAXSERV];

    char msg_buf[255];
    size_t len = 255;

    int er = recvfrom(sockfd, msg_buf, len, MSG_DONTWAIT, (struct sockaddr *) &peer_addr, &peer_addr_len);

    if(er < 0) {
        return new Message(-1,-1,"",1, false);
    }

    Message* m;

    int s = getnameinfo((struct sockaddr *) &peer_addr, peer_addr_len, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
    cout << peer_addr.sin_port << endl;
    if (s == 0) {
        int idSource = -1;

        for(auto p:m_procs) {
            if(p->addrinfo == &peer_addr) {
                cout << "Found match" << endl;
            }
        }

        string payload = string(msg_buf);
        bool ack = false;
        if (payload.find(ACK) != std::string::npos) {
            // message is ack message
            ack = true;
        }

        m = new Message(idSource,curr_proc,string(msg_buf),string(msg_buf).size(), ack);

        cout << "Received :" << *m << endl;
    } else {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
        m = new Message(-1,-1,"",0, false);
    }

    return m;
}


//Stubborn Links Modules
StubbornLinks::StubbornLinks(vector<process *> &procs, int id)
:UDP(procs, id)
{}

int StubbornLinks::send(Message *m) {

    // message is never ack so payload is always the seq number
    vector<int> curr_acks = acks_per_proc[m->sid];
    int seqNum = stringToInt(m->payload);
    while(find(curr_acks.begin(), curr_acks.end(), seqNum ) == curr_acks.end()) {
        UDP::send(m);
    }
    cout << "Stopped sending message : [" << m << "]" << endl;
}

Message* StubbornLinks::rcv(Message *m) {

    m = UDP::rcv(NULL);

    if(m->ack) {
        // payload is of format "ack #"
        string seq = m->payload.substr(3);
        int seqNum = stringToInt(seq);
        acks_per_proc[m->sid].push_back(seqNum);
        cout << acks_per_proc[m->sid][0] << endl;
        // discard message
    } else {
        // send ack
        auto* ackMess = new Message(m->sid, m->did, "ack " + m->payload, m->payload.size() + 4, true);
        UDP::send(ackMess);
        delete ackMess;
        return m;
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
