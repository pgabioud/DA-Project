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
    Protocol* p;
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

    virtual int send(int seq, int dest, int sender) = 0;
    virtual void rcv(Message **message) = 0;

    void broadcast(int seq);
    void deliver(int seq, int os);
    void startSending();

    void init_socket(process* proc) ;


protected:
    vector<pthread_t> threads;
public:
    //Need for perfect links
    vector<set<string>> acks_per_proc;
    set<string> seen;

    //contains messages represented by pair <seq, original sender> for each process
    vector<set<pair<int,int>>> bmessages;

    // stubborn links confirmation container
    vector<set<pair<int,int>>> sl_delivered;
    // perfect links no duplication container
    vector<set<pair<int,int>>> pl_delivered;
    // urb pending messages container and vector clocks for uniformisation
    set<pair<int,int>> pending;
    set<pair<int,int>> urb_delivered;
    vector<vector<int>> vectorClock;


};

class UDP: public Protocol {

public:
    UDP(vector<process*> & processes, int curr_id,int m);
    ~UDP() override;

    int send(int seq, int dest, int sender);
    void rcv(Message **message);

    int sendAck(int seq, int dest, int sender);

};

class StubbornLinks : public UDP {

public:
    StubbornLinks(vector<process*> & processes, int curr_id,int m);
    ~StubbornLinks() override;

    int send(int seq, int dest, int sender);
    void rcv(Message **message);

};

class PerfectLinks : public StubbornLinks {

public:
    PerfectLinks(vector<process*> & processes, int curr_id, int m);
    ~PerfectLinks() override;

    int send(int seq, int dest, int sender);
    void rcv(Message **message);

};


class Urb : public PerfectLinks {

public:
    Urb(vector<process*> & processes, int curr_id, int m);
    ~Urb() override;

    int send(int seq, int dest, int sender);
    void rcv (Message **message);

};


/*
class Fifo : public Urb {

public:
    Fifo(vector<process*> & processes, int curr_id, int m);
    ~Fifo() override;

    int send(int seq, int dest, int sender);
    void rcv(Message** message);

public:
    vector<int> next;
    vector<vector<int>> unorderedMessage;
};

*/
#endif //PROJECT_PROTOCOL_H
