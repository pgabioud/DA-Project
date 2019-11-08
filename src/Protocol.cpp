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
            bmessages[j].insert(i);
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

vector<string> split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
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
        if (payload.find(ACK) != std::string::npos) {
            // message is ack message
            ack = true;
            os = stringToInt(tokens[2]);
            seq = stringToInt(tokens[1]);
        } else {
            os = stringToInt(tokens[1]);
            seq = stringToInt(tokens[0]);
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
        // original sender is me
        if((*m)->os == curr_proc) {
            sl_delivered[(*m)->sid].insert((*m)->seqNum);
        }
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
    int seq = (*m)->seqNum;

    if(!(*m)->discard) {
        auto found = find(pl_delivered[(*m)->sid].begin(), pl_delivered[(*m)->sid].end(), seq);
        if(found == pl_delivered[(*m)->sid].end()) {
            // did not find
            pl_delivered[(*m)->sid].insert((*m)->seqNum);
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
    for(auto& s : pendingMessage) {
        if(s==m->payload) {
            return PerfectLinks::send(m);
        }
    }

    bool is_delivered =false;
    for(auto& s : urb_delivered) {
        if(s==m->payload) {
            is_delivered = true;
            break;
        }
    }

    if(!is_delivered) {
        pendingMessage.insert(m->payload);
    }
    return PerfectLinks::send(m);


}

void Urb::rcv(Message **m) {


    if((*m)== nullptr) {
        //return m;
    }

    int seq = (*m)->seqNum;

    //check if already received message with original sender
    auto found = find(proc_pending[(*m)->os].begin(), proc_pending[(*m)->os].end(), seq);
    if(found != proc_pending[(*m)->os].end()) {
        proc_pending[(*m)->os].insert(seq);
        proc_rebroadcast_queue[(*m)->os].insert(seq);
    }


    //rebroadcast message


    /*
    if((*m)->seqNum > vectorClock[(*m)->os].size()) {
        vectorClock[(*m)->os].resize((*m)->seqNum, 1);
    }

    //receive message
    vectorClock[(*m)->os][iseq] += 1;

    bool is_pending = false;
    double half = num_procs * 0.5;
    bool is_delivered = false;

    for (auto& s:pendingMessage) {
        if (s == (*m)->payload) {
            is_pending = true;
            break;
        }
    };

    if(!is_pending) {
        pendingMessage.insert((*m)->payload);
        is_pending = true;
    }

    for (auto& s:urb_delivered) {
        if (s == (*m)->payload) {
            is_delivered = true;
            break;
        }
    };

    bool can_deliver = vectorClock[(*m)->os][iseq] > half;

    if(can_deliver and is_pending and !is_delivered) {
        pendingMessage.erase((*m)->payload);
        urb_delivered.insert((*m)->payload);
        //return m;
    } else {
        delete m;
        //return nullptr;
    }

     */

}
