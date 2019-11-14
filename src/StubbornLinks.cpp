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
#include "StubbornLinks.h"
#include <mutex>

#define ACK "ack"
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
