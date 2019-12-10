//
// Created by pgabioud on 03/12/2019.
//

#ifndef DA_PROJECT_LCB_H
#define DA_PROJECT_LCB_H

#include "Urb.h"

class LCB : Urb {

public:
    LCB(vector<process*> & processes, int curr_id, int m);
    ~LCB() override;

    int send(int seq, int dest, int sender);
    void rcv(Message **message);

    //bool canDeliver(pair<string, unsigned> key);

    vector<int> vectorClock;

    //pending
    vector<tuple<string, string, string>> pending;

};

#endif //DA_PROJECT_LCB_H