//
// Created by pgabioud on 03/12/2019.
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
#include "LocalizedCausal.h"
#include "Utils.h"

LocalizedCausal::LocalizedCausal(vector<process *> &procs, int id, int m)
:Urb(procs, id,m)
{
}

LocalizedCausal::~LocalizedCausal()
{
}

int LocalizedCausal::send(int seq, int dest, int sender) {
    if(vectorClock[sender] == seq - 1) {
        vectorClock[sender] += 1;
    }
    return Urb::send(seq, dest, sender);
}

void LocalizedCausal::rcv(Message **m) {

    Urb::rcv(m);

    if ((*m) == nullptr) {
        return;
    }
    if ((*m)->discard) {
        return;
    }
/*
    if (find(vectorClock[(*m)->os].begin(), vectorClock[(*m)->os].end(), (*m)->seqNum) == vectorClock[(*m)->os].end()) {
        (*m)->discard = true;
        return;
    }*/

    vector<int> sourceVectorClock(num_procs, 0);
    stringToVectorClock((*m)->strSourceVC, num_procs, &sourceVectorClock);

    for(std::vector<int>::size_type i = 0; i != sourceVectorClock.size(); i++) {

    }


    //deliver((*m)->seqNum, (*m)->os);
}

