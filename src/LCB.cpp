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
#include <iterator>
#include "Protocol.h"
#include "LCB.h"
#include "Utils.h"

LCB::LCB(vector<process *> &procs, int id, int m)
:Urb(procs, id,m)
{
    vectorClock.resize(num_procs);
    lsn = 0;

}

LCB::~LCB()
{
    pending.clear();
}

int LCB::send(int seq, int dest, int sender, string vc) {
    vector<int> W(vectorClock);
    W[curr_proc]= lsn;
    lsn++;

    //call urb broadcasting
    return Urb::send(seq, dest, sender, vectorClockToString(&W));
}

void LCB::rcv(Message **m) {

    Urb::rcv(m);

    if ((*m) == nullptr) {
        return;
    }
    if ((*m)->discard) {
        return;
    }

    //cout <<"LCB received : " << (**m) << endl;
    //deliver((*m)->seqNum, (*m)->os);

    // insert message in pending messages container
    Message key((*m)->sid, (*m)->did, 0, (*m)->os, (*m)->seqNum, "", (*m)->strSourceVC);
    pending.insert(key);

    vector<int> pendingVectorClock(num_procs, 0);

    //attempt delivery
    bool invalid;

    auto m_it = pending.begin();

    while (m_it != pending.end()) {
        if(pending.empty()) break;
        Message curr_m = *m_it;
        stringToVectorClock(curr_m.strSourceVC, &pendingVectorClock);
        invalid = false;
        /*
        cout << "W' : [";
        for(auto v : pendingVectorClock) {
            cout << v << " ";
        }
        cout << "]" <<endl;
        cout << "V : [";
        for(auto v : vectorClock) {
            cout << v << " ";
        }
        cout << "]" <<endl;
*/
        for (int i = 0; i < pendingVectorClock.size(); i++) {
            if (pendingVectorClock[i] > vectorClock[i]) {
                invalid = true;
                break;
            }
        }

        if (!invalid) {
            //message is valid and can deliver, need to restart attempt
            deliver(curr_m.seqNum, curr_m.os);
            pending.erase(m_it);
            m_it = pending.begin();
            vectorClock[(*m)->os] += 1;
        } else {
            //advace iterator to the next message if not valid
            m_it++;
        }
    }

    //cout << "Pending size : " << pending.size() << endl;
    //cout <<"LCB received : " << (**m) << endl;


}
