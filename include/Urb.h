//
// Created by pgabioud on 14/11/2019.
//

#ifndef DA_PROJECT_URB_H
#define DA_PROJECT_URB_H

#include "PerfectLinks.h"


class Urb : public PerfectLinks {

public:
    Urb(vector<process*> & processes, int curr_id, int m);
    ~Urb() override;

    int send(int seq, int dest, int sender);
    void rcv(Message **message);

    bool canDeliver(pair<string, unsigned> key);

public:
    mutex rcv_mtx;
};

#endif //DA_PROJECT_URB_H
