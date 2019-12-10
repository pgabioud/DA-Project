//
// Created by rudra on 01.10.19.
//

#ifndef PROJECT_PROTOCOL_H
#define PROJECT_PROTOCOL_H

#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>
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

// A hash function used to hash a pair of any kind
struct hash_pair {
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2>& p) const
    {
        auto hash1 = hash<T1>{}(p.first);
        auto hash2 = hash<T2>{}(p.second);
        return hash1 ^ hash2;
    }
};

class Protocol {

public:
    vector<process*> m_procs;
    int num_procs;
    int curr_proc;
    string log;
    int numMess = 1;
    int buffSize = 20;
    int buffIndex = 0;
    bool end = false;

public:
    Protocol(vector<process*> & processes, int curr_id, int m);

    virtual ~Protocol();

    virtual int send(int seq, int dest, int sender, string vc = "") = 0;
    virtual void rcv(Message **message) = 0;

    void broadcast(int seq);
    void deliver(int seq, int os);
    void startSending();
    void finish();

    void init_socket(process* proc) ;

    mutex dlvmtx;

protected:
    vector<pthread_t> threads;
public:
    //log
    vector<pair<int,int>> logBuffer;

    //Need for perfect links
    vector<unordered_set<string>> acks_per_proc;
    // perfect links no duplication container
    vector<unordered_set<pair<int,int>, hash_pair>> pl_delivered;

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
