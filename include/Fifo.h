//
// Created by pgabioud on 11/11/2019.
//

#ifndef DA_PROJECT_FIFO_H
#define DA_PROJECT_FIFO_H

#include "PerfectLinks.h"

/*
 * First In First Out module
 */
class Fifo : public Urb {

public:
    Fifo(vector<process*> & processes, int curr_id, int m);
    ~Fifo() override;

    int send(int seq, int dest, int sender);
    void rcv(Message** message);

public:
    //next sequence of message that should be deliver for each process
    vector<int> next;

    //pending message to be delivered
    set<vector<int>> unorderedMessage;
};

#endif //DA_PROJECT_FIFO_H
