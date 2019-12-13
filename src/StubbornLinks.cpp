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
#include <pthread.h>

#include "Protocol.h"
#include "StubbornLinks.h"

//Stubborn Links Modules

static bool finish = false;

void* stubbornRun(void* args) {
    process_send_t* tmp = (process_send_t*) args;
    StubbornLinks* prot = (StubbornLinks*)(tmp->p);
    int did = tmp->did;
    TimeoutInfo t_info;
    long init_size;
    while(!prot->isFinished()) {
        init_size = prot->sl_pending_messages[did].size();
        if(prot->sl_pending_messages[did].empty()) {
            continue;
        }
        auto it = prot->sl_pending_messages[did].cbegin();
        while(it != prot->sl_pending_messages[did].cend()) {
            prot->mtx.lock();
            auto key = (*it);
            high_resolution_clock::time_point now = high_resolution_clock::now();
            t_info = key.second;
            if(t_info.sent) {
                prot->mtx.unlock();
                it = prot->sl_pending_messages[did].erase(it);
                break;
            } else {
                ++it;
            }
            prot->mtx.unlock();
            long time_span = duration_cast<nanoseconds>(now - t_info.timeofsend).count();
            if (time_span > t_info.timeout_) {
                //resend
                prot->UDP::send(key.first.seqNum, key.first.did, key.first.os, key.first.strSourceVC); // UDP send
                key.second.updateTimeout();
            }
        }
    }
    free(args);
    return nullptr;

}

StubbornLinks::StubbornLinks(vector<process *> &procs, int id, int m)
        :UDP(procs, id, m)
{
    run_t = vector<pthread_t>(num_procs);
    sl_pending_messages.resize(num_procs);
    for(int i=0; i < num_procs; i++) {
        if(i == curr_proc) {
            continue;
        }

        process_send_t* arg  = (process_send_t*)malloc(sizeof(process_send_t));
        arg->p = this;
        arg->did = i;
        pthread_create(&run_t[i], NULL, &stubbornRun, (void*)arg);
    }
}

StubbornLinks::~StubbornLinks()
{
    sl_finish = true;
    for(auto t: run_t) {
        pthread_join(t, NULL);
    }
    for(auto v : sl_pending_messages) {
        v.clear();
    }
}

bool StubbornLinks::isFinished() const {
    return sl_finish;
}

int StubbornLinks::send(int seq, int dest, int sender, string vc) {
    //sl_pending[dest].insert(make_pair(seq,sender));
    TimeoutInfo t_info;
    sl_pending_messages[dest].insert({Message(curr_proc, dest,0, sender, seq ,"",vc),t_info});
    return 0;
}
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
        Message key((*m)->sid,(*m)->did,0,(*m)->os, (*m)->seqNum);
        mtx.lock();
        sl_pending_messages[(*m)->sid][key].sent = true;
        mtx.unlock();
        (*m)->discard = true;
        return;

    } else {
        // send ack of the rcv seqnum to the sender of the message containing th original sender of the message
        UDP::sendAck((*m)->seqNum, (*m)->sid, (*m)->os);
        return;
    }
}

