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

    //init logBuffer
    logBuffer.resize(buffSize);

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
    logBuffer[buffIndex] = make_pair(seq,os);
    buffIndex +=1;

    //try to deliver
    if(buffIndex == buffSize) {
        // write to log
        ofstream ofs;
        ofs.open(log, std::ofstream::out | std::ofstream::app);
        if (ofs.is_open()) {
            for(auto& p : logBuffer ) {
                ofs <<  "d " + to_string(p.second) + " " + to_string(p.first) << endl;
            }
        }
        ofs.close();
        buffIndex = 0;
    }
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

void Protocol::finish() {   ofstream ofs;
    ofs.open(log, std::ofstream::out | std::ofstream::app);
    if (ofs.is_open()) {

        for(int i = 0; i < buffIndex; i++) {
            ofs <<  "d " + to_string(logBuffer[i].second) + " " + to_string(logBuffer[i].first) << endl;
        }
        logBuffer.clear();
    }

    ofs.close();

}