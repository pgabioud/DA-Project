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

class Protocol;

/*
 * Sending process structure
 *
 * p = a Protocol object
 * did = destination id
 */
typedef struct{
    Protocol* p;
    int did;
}process_send_t;

/*
 * A hash function used to hash a pair of any kind
 */
struct hash_pair {
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2>& p) const
    {
        auto hash1 = hash<T1>{}(p.first);
        auto hash2 = hash<T2>{}(p.second);
        return hash1 ^ hash2;
    }
};

/*
 * Protocol class
 *
 * num_procs = number of process
 * curr_proc = id of current process
 * log = name of log file
 * numMess = number of message
 * buffSize = size of the log buffer to write into the log file
 * buffIndex = current buffer index
 * end = indicator of the end of write in the logs
 */
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

    /*
     * send message
     *
     * @param: seq = sequence number of message
     * @param: dest = destination id of message
     * @param: sender = sender id
     * @param: vc = vector clock of sender
     */
    virtual int send(int seq, int dest, int sender, string vc = "") = 0;

    /*
     * receiver message
     *
     * @param: message = pointer of the message object in which we store the received message
     */
    virtual void rcv(Message **message) = 0;

    /*
     * broadcast to all process the message
     *
     * @parma seq = sequence number of the message
     */
    void broadcast(int seq);

    /*
     * add the sequence number and original sender id in the logbuffer
     *
     * @param seq = sequence number
     * @param os = id of original sender
     */
    void deliver(int seq, int os);

    /*
     * start the broadcasting of messages
     */
    void startSending();

    /*
     * write the remaining logs in the log buffer
     */
    void finish();

    /*
     * initialize the socket for the corresponding proc
     *
     * @param = process that open the socket
     */
    void init_socket(process* proc) ;

    mutex dlvmtx;

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
    vector<unordered_set<string>> acks_per_proc;
};

#endif //PROJECT_PROTOCOL_H
