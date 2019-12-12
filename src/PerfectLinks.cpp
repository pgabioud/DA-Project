//
// Created by pgabioud on 11/11/2019.
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <algorithm>
#include "Protocol.h"
#include "PerfectLinks.h"
#include <iostream>
//Perfect Links Module
PerfectLinks::PerfectLinks(vector<process *> &procs, int id, int m)
:StubbornLinks(procs, id, m)
{
    pl_delivered.resize(num_procs);

}

PerfectLinks::~PerfectLinks()
{
}

int PerfectLinks::send(int seq, int dest, int sender, string vc) {
    //cout << "PL send from os : [" << sender << "] seq : [" << seq << "] vc : [" <<vc <<  "]" << endl;
    return StubbornLinks::send(seq,dest,sender, vc);
}

void PerfectLinks::rcv(Message **m) {

    StubbornLinks::rcv(m);

    if((*m)== nullptr) {
        return;
    }

    if((*m)->discard) {
        return;
    }


    pair<int,int> rcv = make_pair((*m)->seqNum, (*m)->os);
    auto found = find(pl_delivered[(*m)->sid].begin(), pl_delivered[(*m)->sid].end(), rcv);
    if(found == pl_delivered[(*m)->sid].end()) {
        // did not find
        pl_delivered[(*m)->sid].insert(rcv);
        //deliver((*m)->seqNum, (*m)->os);
        //cout << "PL rcv from os : [" << (*m)->os << "] seq : [" << (*m)->seqNum << "] vc : [" << (*m)->strSourceVC << "]" << endl;

    } else {
        // already found so we discard
        (*m)->discard = true;
    }

}

