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

    if(curr_proc == proc->id) {
        if (bind(fd, (struct sockaddr *) addr, sizeof(*addr)) < 0) {
            perror("bind");
            exit(1);

        }
    }

    proc->addrinfo = addr;
    proc->socket = fd;
}

Protocol::Protocol(vector<process*> &processes, int curr_id, int m)
:m_procs(processes), curr_proc(curr_id), acks_per_proc(processes.size()), delivered(processes.size()), numMess(m)
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
    ss << "da_proc_" << curr_id + 1 << ".out";
    log = ss.str();
    // reset log
    ofstream ofs;
    ofs.open(log, std::ofstream::out | std::ofstream::trunc);
    ofs.close();

}

UDP::UDP(vector<process*> &procs, int id, int m)
:Protocol(procs, id, m)
{}

int Protocol::broadcast() {
    string payload = to_string(seqNum);
    for(auto p : m_procs) {
        if(p->id!= curr_proc) {
            auto* m = new Message(curr_proc, p->id, payload, payload.size(), false);
            send(m);
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
    auto *p = m_procs[message->did];

    int er = sendto(m_procs[curr_proc]->socket, message->payload.c_str(), message->payload.size(), 0, (const sockaddr*)(p->addrinfo), sizeof(*p->addrinfo));
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
        auto* m = new Message(-1,-1,"",1, false);
        m->discard = true;
        return m;
    }

    Message* m;

    int s = getnameinfo((struct sockaddr *) &peer_addr, peer_addr_len, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
    int port =  ntohs(peer_addr.sin_port) ;
    if (s == 0) {
        int idSource = -1;

        for(auto p:m_procs) {
            if(p->port == port) {
                idSource = p->id;
            }
        }

        string payload = string(msg_buf);
        bool ack = false;
        if (payload.find(ACK) != std::string::npos) {
            // message is ack message
            ack = true;
        }

        m = new Message(idSource,curr_proc,string(msg_buf),string(msg_buf).size(), ack);
    } else {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
        m = new Message(-1,-1,"",0, false);
        m->discard = true;
    }

    bzero(msg_buf,strlen(msg_buf));

    return m;
}


//Stubborn Links Modules
StubbornLinks::StubbornLinks(vector<process *> &procs, int id, int m)
:UDP(procs, id, m)
{}

int StubbornLinks::send(Message *m) {

    // message is never ack so payload is always the seq number
    int seqNum = stringToInt(m->payload);
    while(find(acks_per_proc[m->did].begin(), acks_per_proc[m->did].end(), seqNum ) == acks_per_proc[m->did].end()) {
        UDP::send(m);
    }
    //delete ack
    acks_per_proc[m->did].erase(find(acks_per_proc[m->did].begin(), acks_per_proc[m->did].end(), seqNum ));
}

Message* StubbornLinks::rcv(Message *m_) {

    Message* m = UDP::rcv(NULL);
    if(m->discard) {
        //Discard message
        return m;
    }

    if(m->ack) {
        // payload is of format "ack #"
        string seq = m->payload.substr(3);
        int seqNum = stringToInt(seq);
        acks_per_proc[m->sid].insert(seqNum);
        // discard message
        m->discard = true;
        return m;
    } else {
        // send ack
        auto* ackMess = new Message(m->did, m->sid, "ack " + m->payload, m->payload.size() + 4, true);
        UDP::send(ackMess);
        delete ackMess;
        return m;
    }
}


//Perfect Links Module
PerfectLinks::PerfectLinks(vector<process *> &procs, int id, int m)
:StubbornLinks(procs, id, m)
{}

int PerfectLinks::send(Message *message) {
    StubbornLinks::send(message);
}

Message* PerfectLinks::rcv(Message *message) {

    auto* m = StubbornLinks::rcv(nullptr);

    if(!m->discard) {
        int seq = stringToInt(m->payload);
        auto found = find(delivered[m->sid].begin(), delivered[m->sid].end(), seq );
        if(found == delivered[m->sid].end()) {
            delivered[m->sid].insert(seq);
            return m;
        } else {
            //discard
            m->discard = -1;
        }
    }

    return m;
}
