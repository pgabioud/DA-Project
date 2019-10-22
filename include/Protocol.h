//
// Created by rudra on 01.10.19.
//

#ifndef PROJECT_PROTOCOL_H
#define PROJECT_PROTOCOL_H

#include <string>
#include <vector>
#include "Utils.h"

using namespace std;

class Protocol {

public:
    vector<process*> m_procs;
    int curr_proc;
    char * rcv_buffer;
    int seqNum;
public:

    Protocol(vector<process*> & processes, int curr_id);

    virtual int send(const char * msg, size_t size, int  p_id) = 0;
    virtual int rcv(char * msg, size_t size) = 0;

};

class UDP: public Protocol {

public:
    UDP(vector<process*> & processes, int curr_id);

    int send(const char * msg, size_t size, int p_id);
    int rcv(char * msg, size_t size);

};


#endif //PROJECT_PROTOCOL_H
