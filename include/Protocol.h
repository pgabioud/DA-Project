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
    string log;
    int sizeBuffer = 10;

public:
    Protocol(vector<process*> & processes, int curr_id);

    virtual int send(const char * msg, size_t size, int  p_id) = 0;
    virtual Message rcv(char * msg, size_t size) = 0;

    void init_socket(process* proc) ;

    int broadcast();

public:
    //Need for perfect links
    vector<Message> delivered;
};

class UDP: public Protocol {

public:
    UDP(vector<process*> & processes, int curr_id);

    int send(const char * msg, size_t size, int p_id);
    Message rcv(char * msg, size_t size);
};

class StubbornLinks : public UDP {

public:
    StubbornLinks(vector<process*> & processes, int curr_id);

    int send(const char * msg, size_t size, int p_id);
    Message rcv(char * msg, size_t size);

private:
    double timeout = 3.0;
};


class PerfectLinks : public StubbornLinks {

public:
    PerfectLinks(vector<process*> & processes, int curr_id);

    int send(const char * msg, size_t size, int p_id);
    Message rcv(char * msg, size_t size);
};


#endif //PROJECT_PROTOCOL_H
