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
#include "LCB.h"
#include "Utils.h"

LCB::LCB(vector<process *> &procs, int id, int m)
:Urb(procs, id,m)
{
    vectorClock.resize(num_procs);
}

LCB::~LCB()
{
}

int LCB::send(int seq, int dest, int sender) {
    if(!(find(m_procs[curr_proc]->affectedProcess.begin(), m_procs[curr_proc]->affectedProcess.end(), dest)
         != m_procs[curr_proc]->affectedProcess.end())) {
        return 0;
    }
    //vectorClock[sender] += 1;
    return Urb::send(seq, dest, sender);
}

void LCB::rcv(Message **m) {

    Urb::rcv(m);

    if ((*m) == nullptr) {
        return;
    }
    if ((*m)->discard) {
        return;
    }

    pending.push_back(make_tuple(to_string((*m)->os),(*m)->strSourceVC, to_string((*m)->seqNum)));

    vector<int> sourceVectorClock(num_procs, 0);
    stringToVectorClock((*m)->strSourceVC, &sourceVectorClock);

    vector<int> pendingVectorClock(num_procs, 0);

    bool findSomething = true;
    while (findSomething) {
        findSomething = false;
        for(std::vector<int>::size_type i = 0; i != pending.size(); i++) {
            stringToVectorClock(get<1>(pending[i]), &pendingVectorClock);
            bool inf = true;
            for(auto const& affectProc: m_procs[curr_proc]->affectedProcess) {
                if(pendingVectorClock[affectProc] > vectorClock[affectProc]) {
                    inf = false;
                }
                if(inf) {
                    findSomething = true;
                    pending.erase(pending.begin()+i);
                    vectorClock[affectProc] += 1;
                    deliver(stringToInt(get<2>(pending[i])), affectProc);
                }
            }
        }
    }
}