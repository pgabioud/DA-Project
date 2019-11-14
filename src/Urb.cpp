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
#include <mutex>

Urb::Urb(vector<process *> &procs, int id, int m)
        :PerfectLinks(procs, id,m)
{
}

Urb::~Urb()
{
}

int Urb::send(int seq, int dest, int sender) {
    //cout << "Sending " << seq << " " << sender << endl;
    pending.insert(make_pair(seq, dest));

    return PerfectLinks::send(seq, dest, sender);
}
mutex pendingLock;
void Urb::rcv(Message **m) {

    PerfectLinks::rcv(m);

    if((*m)== nullptr) {
        return;
    }
    if((*m)->discard) {
        return;
    }


    int iseq = (*m)->seqNum - 1;
    //receive message
    vectorClock[(*m)->os][iseq] += 1;

    double half = num_procs * 0.5;
    bool is_delivered = false;

    //check if already received message with original sender
    pair<int,int> rcv = make_pair((*m)->seqNum, (*m)->os);

    pendingLock.lock();
    auto found = find(pending.begin(), pending.end(), rcv);
    if(found == pending.end()) {
        pending.insert(rcv);

        // rebroadcasting which means adding to all bmessages containers for each process
        for(int i = 0 ; i < num_procs; i++){
            if(i != curr_proc){
                bmessages[i].insert(rcv);
            }
        }
    }
    pendingLock.unlock();

    auto delivered = find(urb_delivered.begin(), urb_delivered.end(), rcv);
    is_delivered = delivered != urb_delivered.end();

    bool can_deliver = vectorClock[(*m)->os][iseq] > half;

    if(can_deliver  and !is_delivered) {
        urb_delivered.insert(rcv);
        cout << "URB delivered : [" << rcv.first << " " << rcv.second << "]" << endl;
    } else {
        (*m)->discard = true;
    }

}