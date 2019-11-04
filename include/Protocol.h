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

public:
    Protocol(vector<process*> & processes, int curr_id);

    virtual int send(Message *message) = 0;
    virtual Message* rcv(Message *message) = 0;

    void init_socket(process* proc) ;

    int broadcast();

public:
    //Need for perfect links
    vector<set<int>> delivered;
    vector<set<int>> acks_per_proc;

};

class UDP: public Protocol {

public:
    UDP(vector<process*> & processes, int curr_id);

    int send(Message *message);
    Message* rcv(Message *message);
};

class StubbornLinks : public UDP {

public:
    StubbornLinks(vector<process*> & processes, int curr_id);

    int send(Message *message);
    Message* rcv(Message *message);

private:
    double timeout = 3.0;
};


class PerfectLinks : public StubbornLinks {

public:
    PerfectLinks(vector<process*> & processes, int curr_id);

    int send(Message *message);
    Message* rcv(Message *message);
};


#endif //PROJECT_PROTOCOL_H
