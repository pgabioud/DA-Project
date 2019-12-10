//
// Created by pgabioud on 14/11/2019.
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
#include "Urb.h"

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

void Urb::rcv(Message **m) {

    PerfectLinks::rcv(m);

    if((*m) == nullptr) {
        return;
    }
    if((*m)->discard) {
        return;
    }

    //cout << "URB received : ["<< (*m)->payload << "]" << endl;

    pair<string, int> mRcv = make_pair((*m)->payload, (*m)->os);

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
            //cout <<"Rebroadcasting : [" <<  (*m)->seqNum << " " <<  (*m)->os <<"]" << endl;

            if(j != curr_proc) {
                PerfectLinks::send((*m)->seqNum, j, (*m)->os);
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
        deliver((*m)->seqNum, (*m)->os);
    } else {
        (*m)->discard = true;
    }
}
