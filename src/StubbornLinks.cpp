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
    //vector<pair<int,int>> messages_copy;
    vector<Message> messages_copy;

    while(!prot->isFinished()) {
        if(prot->sl_pending_messages[did].empty()) {
            messages_copy.clear();
            continue;
        }
        // Copying vector by copy function
        //copy(prot->sl_pending[did].begin(), prot->sl_pending[did].end(), back_inserter(messages_copy));
        copy(prot->sl_pending_messages[did].begin(), prot->sl_pending_messages[did].end(), back_inserter(messages_copy));

        for(auto it : messages_copy) {
            prot->UDP::send(it.seqNum, it.did, it.os, it.strSourceVC); // UDP send

            nanosleep((const struct timespec[]){{0, 1000000000L}}, NULL);
        }
        messages_copy.clear();
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
}

bool StubbornLinks::isFinished() const {
    return sl_finish;
}

int StubbornLinks::send(int seq, int dest, int sender, string vc) {
    //sl_pending[dest].insert(make_pair(seq,sender));
    sl_pending_messages[dest].insert(Message(curr_proc, dest,0, sender, seq ,"",vc));
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
        pair<int,int> rcv = make_pair((*m)->seqNum, (*m)->os) ;
        Message key((*m)->sid,(*m)->did,0,(*m)->os, (*m)->seqNum);

        sl_pending_messages[(*m)->sid].erase(key);

        (*m)->discard = true;
        return;

    } else {
        // send ack of the rcv seqnum to the sender of the message containing th original sender of the message
        UDP::sendAck((*m)->seqNum, (*m)->sid, (*m)->os);
        return;
    }
}

