//
// Created by rudra on 01.10.19.
//

#ifndef PROJECT_PROTOCOL_H
#define PROJECT_PROTOCOL_H

#include <string>
#include <vector>
#include <set>
#include "Utils.h"

using namespace std;

static vector<vector<string>> logBuffer;

void* single_send(void* args);

void* broadcast_to_p(void* args);

class Protocol {

public:
    vector<process*> m_procs;
    int num_procs;
    int curr_proc;
    int seqNum;
    string log;
    int sizeBuffer = 50;
    int numMess = 1;

public:
    Protocol(vector<process*> & processes, int curr_id, int m);

    virtual ~Protocol();

    virtual int send(Message *message) = 0;
    virtual Message* rcv(Message *message) = 0;

    void init_socket(process* proc) ;

    int broadcast();
protected:
    vector<pthread_t> threads;
public:
    //Need for perfect links
    vector<set<string>> acks_per_proc;
    set<string> seen;

};

class UDP: public Protocol {

public:
    UDP(vector<process*> & processes, int curr_id,int m);
    ~UDP() override;

    int send(Message *message);
    Message* rcv(Message *message);
};

class StubbornLinks : public UDP {

public:
    StubbornLinks(vector<process*> & processes, int curr_id,int m);
    ~StubbornLinks() override;

    int send(Message *message);
    Message* rcv(Message *message);

};


class PerfectLinks : public StubbornLinks {

public:
    PerfectLinks(vector<process*> & processes, int curr_id, int m);
    ~PerfectLinks() override;

    int send(Message *message);
    Message* rcv(Message *message);

    vector<set<string>> pl_delivered;

};

class Urb : public PerfectLinks {

public:
    Urb(vector<process*> & processes, int curr_id, int m);
    ~Urb() override;

    int send(Message *message);
    Message * rcv (Message *message);

public:
    vector<vector<int>> vectorClock;
    set<string> pendingMessage;
    set<string> urb_delivered;
};

typedef struct{
    PerfectLinks* prot;
    Message* m;
    int did;
}pl_send_args;

typedef struct{
    Protocol* prot;
    Message* m;
    int did;
}send_args;

#endif //PROJECT_PROTOCOL_H
