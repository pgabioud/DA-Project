//
// Created by pgabioud on 03/12/2019.
//

#ifndef DA_PROJECT_LCB_H
#define DA_PROJECT_LCB_H

#include "Urb.h"

/*
 * Localized Reliable Broadcast module
 */
class LCB : public Urb {

public:
    LCB(vector<process*> & processes, int curr_id, int m);
    ~LCB() override;

    int send(int seq, int dest, int sender, string vc = "");
    void rcv(Message **message);

    // Vector Clock
    vector<int> vectorClock;

    // counter of messages
    float lsn;

    //pending messages
    unordered_set<Message, hash_message> pending;


};

#endif //DA_PROJECT_LCB_H