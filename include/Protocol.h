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

class Protocol {

public:
    vector<process*> m_procs;
    int curr_proc;
    char * rcv_buffer;
    int seqNum;
    string log;
    int sizeBuffer = 10;
    int numMess = 1;

public:
    Protocol(vector<process*> & processes, int curr_id, int m);

    virtual int send(Message *message) = 0;
    virtual Message* rcv(Message *message) = 0;

    void init_socket(process* proc) ;

    int broadcast();
protected:
    vector<pthread_t> threads;
public:
    //Need for perfect links
    vector<set<string>> delivered;
    vector<set<string>> acks_per_proc;
    set<string> seen;

};

class UDP: public Protocol {

public:
    UDP(vector<process*> & processes, int curr_id,int m);

    int send(Message *message);
    Message* rcv(Message *message);
};

class StubbornLinks : public UDP {

public:
    StubbornLinks(vector<process*> & processes, int curr_id,int m);

    int send(Message *message);
    Message* rcv(Message *message);

private:
    double timeout = 3.0;
};


class PerfectLinks : public StubbornLinks {

public:
    PerfectLinks(vector<process*> & processes, int curr_id, int m);

    int send(Message *message);
    Message* rcv(Message *message);
};

typedef struct{
    PerfectLinks* prot;
    Message* m;
}send_args;

#endif //PROJECT_PROTOCOL_H
