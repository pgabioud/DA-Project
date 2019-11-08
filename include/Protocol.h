//
// Created by rudra on 01.10.19.
//

#ifndef PROJECT_PROTOCOL_H
#define PROJECT_PROTOCOL_H

#include <string>
#include <vector>
#include <set>
#include <deque>
#include "Utils.h"

using namespace std;

static vector<vector<string>> logBuffer;

void* single_send(void* args);

void* broadcast_to_p(void* args);

class Protocol;
class UDP;
class StubbornLinks;
class PerfectLinks;
class Urb;
typedef struct{
    Urb* p;
    int did;
}process_send_t;


class Protocol {

public:
    vector<process*> m_procs;
    int num_procs;
    int curr_proc;
    int seqNum;
    string log;
    int sizeBuffer = 10;
    int numMess = 1;
    int max_try = 5;
    int max_send = 50;
    int curr_seq = 0;

public:
    Protocol(vector<process*> & processes, int curr_id, int m);

    virtual ~Protocol();

    virtual int send(Message *message) = 0;
    virtual void rcv(Message **message) = 0;



    void init_socket(process* proc) ;

    int broadcast();

protected:
    vector<pthread_t> threads;
public:
    //Need for perfect links
    vector<set<string>> acks_per_proc;
    set<string> seen;

    vector<set<string>> bmessages;
    vector<set<int>> rebroadcasts;

    vector<int> proc_counters;

};

class UDP: public Protocol {

public:
    UDP(vector<process*> & processes, int curr_id,int m);
    ~UDP() override;

    int send(Message *message);
    void rcv(Message **message);
};

class StubbornLinks : public UDP {

public:
    StubbornLinks(vector<process*> & processes, int curr_id,int m);
    ~StubbornLinks() override;

    int send(Message *message);
    void rcv(Message **message);

    vector<set<string>> sl_delivered;

};


class PerfectLinks : public StubbornLinks {

public:
    PerfectLinks(vector<process*> & processes, int curr_id, int m);
    ~PerfectLinks() override;

    int send(Message *message);
    void rcv(Message **message);

    vector<set<string>> pl_delivered;

};

class Urb : public PerfectLinks {

public:
    Urb(vector<process*> & processes, int curr_id, int m);
    ~Urb() override;

    int send(Message *message);
    void rcv (Message **message);

public:
    vector<vector<int>> vectorClock;
    set<string> pendingMessage;
    set<string> urb_delivered;

    vector<set<string>> proc_rebroadcast_queue;
    vector<set<string>> proc_pending;
};




#endif //PROJECT_PROTOCOL_H
