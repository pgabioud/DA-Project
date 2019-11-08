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
#include <mutex>

#define ACK "ack"

void* work(void* arg);

mutex mtx;


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
:m_procs(processes), num_procs(processes.size()),curr_proc(curr_id), acks_per_proc(num_procs), numMess(m)

{
    for(auto & p: m_procs) {
            init_socket(p);
    }
    cout << "Sockets inited" << endl;
    for(auto& p:m_procs) {
        cout << *p << endl;
    }

    //init variables
    //init vectors
    seen = set<string>();
    threads = vector<pthread_t>();
    seqNum = 1;


    stringstream ss;
    ss << "da_proc_" << curr_id + 1 << ".out";
    log = ss.str();
    // reset log
    ofstream ofs;
    ofs.open(log, std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    proc_counters.resize(num_procs, 0);
    rebroadcasts.resize(num_procs);
    bmessages.resize(num_procs);
    for(int j= 0; j < num_procs;j++) {
        for(int i = 1; i <=numMess;i++) {
            //create payload
            string payload = to_string(i) + " " +  to_string(curr_proc) + " " + to_string(j);
            bmessages[j].insert(payload);
        }
    }

    // create thread for working


}


Protocol::~Protocol()
{

}

UDP::UDP(vector<process*> &procs, int id, int m)
:Protocol(procs, id, m)
{}


UDP::~UDP()
{}

int Protocol::broadcast() {
}


int UDP::send(Message *message) {
    process *p = m_procs[message->did];

    int er = sendto(m_procs[curr_proc]->socket, message->payload.c_str(), message->payload.size(), 0, (const sockaddr*)(p->addrinfo), sizeof(*p->addrinfo));
    if(er < 0) {
        cerr << "Error sending message : " << message << endl;
    }

    return er;
}


void UDP::rcv(Message **m) {
    int sockfd = m_procs[curr_proc]->socket;
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len;
    peer_addr_len = sizeof(peer_addr);
    char host[NI_MAXHOST], service[NI_MAXSERV];

    char msg_buf[255];
    size_t len = 255;

    int er = recvfrom(sockfd, msg_buf, len, MSG_DONTWAIT, (struct sockaddr *) &peer_addr, &peer_addr_len);

    if(er < 0) {
        return;
    }


    int s = getnameinfo((struct sockaddr *) &peer_addr, peer_addr_len, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
    int port =  ntohs(peer_addr.sin_port) ;
    if (s == 0) {
        int idSource = -1;

        for(auto& p:m_procs) {
            if(p->port == port) {
                idSource = p->id;
            }
        }

        string payload = string(msg_buf);
        auto tokens = split(payload,' ');
        bool ack = false;
        int os = -1;
        int seq = -1;
        int sid = -1;
        if (payload.find(ACK) != std::string::npos) {
            // message is ack message
            ack = true;
            sid = stringToInt(tokens[3]);
            os = stringToInt(tokens[2]);
            seq = stringToInt(tokens[1]);
        } else {
            os = stringToInt(tokens[1]);
            seq = stringToInt(tokens[0]);
            sid = stringToInt(tokens[2]);
        }
        *m=new Message(idSource,curr_proc, ack, os, seq);
        (*m)->discard = false;
    } else {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
        m = nullptr;
    }

    bzero(msg_buf,strlen(msg_buf));

}


//Stubborn Links Modules
StubbornLinks::StubbornLinks(vector<process *> &procs, int id, int m)
:UDP(procs, id, m)
{
    sl_delivered.resize(num_procs);
}

StubbornLinks::~StubbornLinks()
{
}

int StubbornLinks::send(Message *m) {

    return UDP::send(m);
}

void StubbornLinks::rcv(Message **m) {

    UDP::rcv(m);

    if ((*m) == nullptr) {
        //discard
        return;
    }

    if((*m)->ack) {
        // payload is of format "ack # #"
        string payload = (*m)->payload.substr(4);
        cout << "payload : [" << payload << "]"<< endl;
        sl_delivered[(*m)->sid].insert(payload);

        (*m)->ack = true;
        (*m)->discard = true;
        return;

    } else {
        // send ack
        Message* ackMess = new Message((*m)->did, (*m)->sid,  true, (*m)->os, (*m)->seqNum);
        UDP::send(ackMess);
        delete ackMess;
        return;
    }
}


//Perfect Links Module
PerfectLinks::PerfectLinks(vector<process *> &procs, int id, int m)
:StubbornLinks(procs, id, m)
{
    pl_delivered.resize(num_procs);

}

PerfectLinks::~PerfectLinks()
{
}

int PerfectLinks::send(Message *message) {
    return StubbornLinks::send(message);
}



void PerfectLinks::rcv(Message **m) {

    StubbornLinks::rcv(m);

    if((*m)== nullptr) {
        return;
    }

    string payload((*m)->payload);

    if(!(*m)->discard) {
        auto found = find(pl_delivered[(*m)->sid].begin(), pl_delivered[(*m)->sid].end(), payload);
        if(found == pl_delivered[(*m)->sid].end()) {
            // did not find
            pl_delivered[(*m)->sid].insert(payload);
        } else {
            (*m)->discard = true;
            return;
        }
    }


}


Urb::Urb(vector<process *> &procs, int id, int m)
:PerfectLinks(procs, id,m)
{
    vectorClock.resize(num_procs, vector<int>(m, 1));
    proc_rebroadcast_queue.resize(num_procs);
    proc_pending.resize(num_procs);
}

Urb::~Urb()
{
}

int Urb::send(Message *m) {
    proc_pending[m->os].insert(m->payload);
    return PerfectLinks::send(m);

}

void Urb::rcv(Message **m) {

    PerfectLinks::rcv(m);

    if((*m)== nullptr) {
        return;
    }

    string payload((*m)->payload);

    cout << "URB receive :" << (*m)->payload << endl;

    int iseq = (*m)->seqNum - 1;
    //receive message
    vectorClock[(*m)->os][iseq] += 1;

    double half = num_procs * 0.5;
    bool is_delivered = false;

    //check if already received message with original sender
    auto found = find(proc_pending[(*m)->os].begin(), proc_pending[(*m)->os].end(), payload);
    if(found == proc_pending[(*m)->os].end()) {
        proc_pending[(*m)->os].insert(payload);
        proc_rebroadcast_queue[(*m)->os].insert(payload);

        cout << "Added to rebroadcast :" << payload << endl;
    }

    for (auto& s:urb_delivered) {
        if (s == (*m)->payload) {
            is_delivered = true;
            break;
        }
    };

    bool can_deliver = vectorClock[(*m)->os][iseq] > half;

    if(can_deliver  and !is_delivered) {
        urb_delivered.insert(payload);
    } else {
        (*m)->discard = true;
    }

    return;

}
