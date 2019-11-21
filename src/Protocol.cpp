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
    for(auto& p :m_procs) {
        delete p;
    }
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
