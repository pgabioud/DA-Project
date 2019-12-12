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
#include <unistd.h>

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


    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 5;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

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
}

Protocol::~Protocol()
{
    //Network cleanup
    for(auto& p :m_procs) {
        close(p->socket);
        free(p->addrinfo);
        delete p;
    }
 }

void Protocol::startSending() {
    for(int i = 1; i <=numMess;i++) {
        broadcast(i);
        send(i, 0, curr_proc);
    }
}

void Protocol::deliver(int seq, int os) {
    dlvmtx.lock();
    logBuffer[buffIndex] = "d " + to_string(os + 1) + " " + to_string(seq);
    buffIndex +=1;

    //try to deliver
    if(buffIndex >= buffSize) {
        // write to log
        ofstream ofs;
        ofs.open(log, std::ofstream::out | std::ofstream::app);
        if (ofs.is_open()) {
            for(auto& p : logBuffer ) {
                ofs << p << endl;
            }
        }
        ofs.close();
        buffIndex = 0;
    }
    dlvmtx.unlock();

}

void Protocol::broadcast(int seq) {
    dlvmtx.lock();
    // write to log
    logBuffer[buffIndex] = ("b " + to_string(seq));
    buffIndex += 1;

    //try to deliver
    if(buffIndex >= buffSize) {
        // write to log
        ofstream ofs;
        ofs.open(log, std::ofstream::out | std::ofstream::app);
        if (ofs.is_open()) {
            for(auto& p : logBuffer ) {
                ofs << p << endl;
            }
        }
        ofs.close();
        buffIndex = 0;
    }
    dlvmtx.unlock();
}

void Protocol::finish() {   ofstream ofs;
    end = true;

    ofs.open(log, std::ofstream::out | std::ofstream::app);
    if (ofs.is_open()) {
        for(int i = 0; i < buffIndex; i++) {
            ofs <<  logBuffer[i] << endl;
        }
        logBuffer.clear();
    }
    ofs.close();


}