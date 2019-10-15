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
public:

    Protocol(vector<process*> & processes, int curr_id);

    virtual int send(const char * msg, size_t size, process * p) = 0;
    virtual int rcv(char * msg, size_t size, process * p) = 0;

};

class UDP: public Protocol {

public:
    UDP(vector<process*> & processes, int curr_id);

    int send(const char * msg, size_t size, process* p);
    int rcv(char * msg, size_t size, process * p);

};


#endif //PROJECT_PROTOCOL_H
