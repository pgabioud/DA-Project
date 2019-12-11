//
// Created by pgabioud on 03/12/2019.
//

#ifndef DA_PROJECT_LCB_H
#define DA_PROJECT_LCB_H

#include "Urb.h"

class LCB : public Urb {

public:
    LCB(vector<process*> & processes, int curr_id, int m);
    ~LCB() override;

    int send(int seq, int dest, int sender, string vc = "");
    void rcv(Message **message);

    //bool canDeliver(pair<string, unsigned> key);

    vector<int> vectorClock;
    float lsn;
    unordered_set<Message, hash_message> pending;


};

#endif //DA_PROJECT_LCB_H