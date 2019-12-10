//
// Created by pgabioud on 11/11/2019.
//

#ifndef DA_PROJECT_STUBBORNLINKS_H
#define DA_PROJECT_STUBBORNLINKS_H

#include "UDP.h"


using namespace std;

// A hash function used to hash a pair of any kind
struct hash_message {
    size_t operator()(const Message& m) const
    {
        auto hash1 = hash<int>{}(m.seqNum);
        auto hash2 = hash<int>{}(m.os);
        return hash1 ^ hash2;
    }
};

class StubbornLinks : public UDP {

public:
    StubbornLinks(vector<process*> & processes, int curr_id,int m);
    virtual ~StubbornLinks() override;

    int send(int seq, int dest, int sender, string vc = "");
    void rcv(Message **message);

    bool isFinished() const;
private:
    vector<pthread_t> run_t;
    bool sl_finish = false;
public:
    // vector containing message sets per process to send
    vector<unordered_set<pair<int,int>, hash_pair>> sl_pending;
    //second attempt
    vector<unordered_set<Message, hash_message>> sl_pending_messages;

};


#endif //DA_PROJECT_STUBBORNLINKS_H
