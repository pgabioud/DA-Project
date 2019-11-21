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

    bmessages.resize(num_procs);
    // SL init
    sl_delivered.resize(num_procs);
    // PL init
    pl_delivered.resize(num_procs);
    // URB init


}

Protocol::~Protocol()
{

}

void Protocol::startSending() {
    for(int i = 1; i <=numMess;i++) {
        broadcast(i);
        for(int j= 0; j < num_procs;j++) {
            //create original messages
            bmessages[j].insert(make_pair(i,curr_proc));
        }
    }
    
}

UDP::UDP(vector<process*> &procs, int id, int m)
:Protocol(procs, id, m)
{}


UDP::~UDP()
{}

void Protocol::deliver(int seq, int os) {
    // write to log
    vector<string> newLog = {"d", to_string((os+ 1)), to_string(seq)};
    ofstream ofs;
    ofs.open(log, std::ofstream::out | std::ofstream::app);
    if (ofs.is_open()) {
        ofs << newLog[0] + " "+newLog[1] +" "+newLog[2] << endl;
    }
    ofs.close();
}

void Protocol::broadcast(int seq) {
// write to log
    vector<string> newLog = {"b",  to_string(seq)};
    ofstream ofs;
    ofs.open(log, std::ofstream::out | std::ofstream::app);
    if (ofs.is_open()) {
        ofs << newLog[0] + " "+newLog[1] << endl;
    }
    ofs.close();
}


int UDP::send(int seq, int dest, int sender) {
    process *p = m_procs[dest];
    string payload = to_string(seq) + " " + to_string(sender);

    int er = sendto(m_procs[curr_proc]->socket, payload.c_str(), payload.size(), 0, (const sockaddr*)(p->addrinfo), sizeof(*p->addrinfo));
    if(er < 0) {
        cerr << "Error sending message : " << payload << " to p" << dest + 1 << endl;
    }

    return er;
}

int UDP::sendAck(int seq, int dest, int sender) {
    process *p = m_procs[dest];
    string payload ="ack " + to_string(seq) + " " + to_string(sender);

    int er = sendto(m_procs[curr_proc]->socket, payload.c_str(), payload.size(), 0, (const sockaddr*)(p->addrinfo), sizeof(*p->addrinfo));
    if(er < 0) {
        cerr << "Error sending message : " << payload << " to p" << dest + 1 << endl;
    }

}

void UDP::rcv(Message **m) {
    int sockfd = m_procs[curr_proc]->socket;
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len;
    peer_addr_len = sizeof(peer_addr);
    char host[NI_MAXHOST], service[NI_MAXSERV];

    char msg_buf[255];
    size_t len = 255;

    int er = recvfrom(sockfd, msg_buf, len, 0, (struct sockaddr *) &peer_addr, &peer_addr_len);

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
        int type = -1;
        int os = -1;
        int seq = -1;
        if (payload.find(ACK) != std::string::npos) {
            // message is ack message
            type = 1;
            os = stringToInt(tokens[2]);
            seq = stringToInt(tokens[1]);
        } else {
            type = 0;
            os = stringToInt(tokens[1]);
            seq = stringToInt(tokens[0]);

        }

        *m=new Message(idSource,curr_proc, type, os, seq);
        (*m)->discard = false;
    } else {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
        (*m) = nullptr;
    }

    bzero(msg_buf,strlen(msg_buf));

}


//Stubborn Links Modules
StubbornLinks::StubbornLinks(vector<process *> &procs, int id, int m)
:UDP(procs, id, m)
{
}

StubbornLinks::~StubbornLinks()
{
}

int StubbornLinks::send(int seq, int dest, int sender) {
    return UDP::send(seq,dest,sender);
}

mutex sl_lock;

void StubbornLinks::rcv(Message **m) {

    UDP::rcv(m);

    if ((*m) == nullptr) {
        //discard
        return;
    }
    if((*m)->discard) {
        return;
    }

    if((*m)->type == 1) {
        // payload is of format "ack # #"
        pair<int,int> rcv = make_pair((*m)->seqNum, (*m)->os) ;
        sl_lock.lock();
        sl_delivered[(*m)->sid].insert(rcv);
        sl_lock.unlock();
        (*m)->discard = true;
        return;

    } else {
        // send ack of the rcv seqnum to the sender of the message containing th original sender of the message
        UDP::sendAck((*m)->seqNum, (*m)->sid, (*m)->os);
        return;
    }
}

//Perfect Links Module
PerfectLinks::PerfectLinks(vector<process *> &procs, int id, int m)
:StubbornLinks(procs, id, m)
{

}

PerfectLinks::~PerfectLinks()
{
}

int PerfectLinks::send(int seq, int dest, int sender) {
    return StubbornLinks::send(seq,dest,sender);
}

mutex plLock;

void PerfectLinks::rcv(Message **m) {

    StubbornLinks::rcv(m);

    if((*m)== nullptr) {
        return;
    }

    if((*m)->discard) {
        return;
    }

    pair<int,int> rcv = make_pair((*m)->seqNum, (*m)->os);
    plLock.lock();
    auto found = find(pl_delivered[(*m)->sid].begin(), pl_delivered[(*m)->sid].end(), rcv);
    if(found == pl_delivered[(*m)->sid].end()) {
        // did not find
        pl_delivered[(*m)->sid].insert(rcv);
        //cout << "PL Delivered : [" << rcv.first << " " << rcv.second << "] from : P"<< (*m)->sid + 1 << endl;

    } else {
        // already found so we discard
        (*m)->discard = true;
    }

    plLock.unlock();

}


Urb::Urb(vector<process *> &procs, int id, int m)
:PerfectLinks(procs, id,m)
{
}

Urb::~Urb()
{
}

bool Urb::canDeliver(pair<string, unsigned> key) {
    return num_procs * 0.5 < ack[key].size();
}

int Urb::send(int seq, int dest, int sender) {
    return PerfectLinks::send(seq, dest, sender);
}
mutex pendingLock;
void Urb::rcv(Message **m) {

    PerfectLinks::rcv(m);

    if((*m) == nullptr) {
        return;
    }
    if((*m)->discard) {
        return;
    }

    cout << "URB received : ["<< (*m)->payload << "]" << endl;

    pair<string, int> mRcv = make_pair((*m)->payload, (*m)->os);

    lock_guard<mutex> guard(rcv_mtx);

    //check if message already delivered
    auto dlvrd = delivered.find(mRcv);
    if(dlvrd != delivered.end()) {
        // already found
        (*m)->discard = true;
        return;
    }

    // not yet delivered
    // check if first time received
    auto seen = ack.find(mRcv);
    if(seen == ack.end()) {
        // did not see so add to rebroadcasting
        for(int j= 0; j < num_procs;j++) {
            //create original messages
            if(j != curr_proc) {
                bmessages[j].insert(make_pair((*m)->seqNum, (*m)->os));
            }
        }

        // create ack entry
        ack[mRcv] = set<int>();
    }

    // at this point myself and sender have seen this message
    ack[mRcv].insert(curr_proc);
    ack[mRcv].insert((*m)->sid);

    if(canDeliver(mRcv)) {
        delivered.insert(mRcv);
    } else {
        (*m)->discard = true;
    }
}

/*
Fifo::Fifo(vector<process *> &procs, int id, int m)
        :Urb(procs, id, m)
{
    next.resize(num_procs, 1);
}

Fifo::~Fifo()
{}

int Fifo::send(Message *m) {
    return Urb::send(m);
}

void Fifo::rcv(Message **m) {
    PerfectLinks::rcv(m);

    if((*m)== nullptr || (*m)->discard) {
        return;
    }

    vector<int> seqOs = {(*m)->seqNum, (*m)->os};
    unorderedMessage.push_back(seqOs);
    bool iterateAgain;
    vector<vector<int>> msgToDelete;

    do {
        iterateAgain = false;
        vector<vector<int>>::iterator it;
        for (it = unorderedMessage.begin(); it != unorderedMessage.end(); it++) {
            int counter = next[(*it)[1]];
            if (counter == (*it)[0]) {
                iterateAgain = true;
                next[(*it)[1]] = counter + 1;
                deliver(*it);
                msgToDelete.push_back((*it));
            }
        }
    } while (iterateAgain);

    for(auto& elem: msgToDelete) {
        auto deliveredMsg = find(unorderedMessage.begin(), unorderedMessage.end(), elem);
        unorderedMessage.erase(deliveredMsg);
    }
    return;
}

 */
