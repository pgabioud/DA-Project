//
// Created by rudra on 01.10.19.
//

#ifndef PROJECT_PROTOCOL_H
#define PROJECT_PROTOCOL_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include <mutex>
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
    string log;
    int numMess = 1;
    int buffSize = 100;
    int buffIndex = 0;

public:
    Protocol(vector<process*> & processes, int curr_id, int m);

    virtual ~Protocol();

    virtual int send(int seq, int dest, int sender) = 0;
    virtual void rcv(Message **message) = 0;

    void broadcast(int seq);
    void deliver(int seq, int os);
    void startSending();
    void finish();

    void init_socket(process* proc) ;


protected:
    vector<pthread_t> threads;
public:
    //vector clock
    vector<int> vectorClock;

    //pending
    vector<pair<string, string>> pending;

    //log
    vector<pair<int,int>> logBuffer;

    //Need for perfect links
    vector<set<string>> acks_per_proc;
    set<string> seen;

    //contains messages represented by pair <seq, original sender> for each process
    vector<set<pair<int,int>>> bmessages;

    // stubborn links confirmation container
    vector<set<pair<int,int>>> sl_delivered;
    // perfect links no duplication container
    vector<set<pair<int,int>>> pl_delivered;
    //Uniform reliable broadacast
    set<pair<string, int>> delivered;
    set<pair<string, int> > ready_for_delivery;
    // ack messages
    map<pair<string, int>, set<int> > ack;
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
