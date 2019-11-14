//
// Created by pgabioud on 11/11/2019.
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
#include "PerfectLinks.h"
#include <mutex>

#define ACK "ack"

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
