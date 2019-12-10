//
// Created by pgabioud on 11/11/2019.
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <algorithm>
#include "Protocol.h"
#include "PerfectLinks.h"

//Perfect Links Module
PerfectLinks::PerfectLinks(vector<process *> &procs, int id, int m)
:StubbornLinks(procs, id, m)
{
    pl_delivered.resize(num_procs);

}

PerfectLinks::~PerfectLinks()
{
}

int PerfectLinks::send(int seq, int dest, int sender) {
    return StubbornLinks::send(seq,dest,sender);
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

    } else {
        // already found so we discard
        (*m)->discard = true;
    }


}

