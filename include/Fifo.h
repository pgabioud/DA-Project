//
// Created by pgabioud on 11/11/2019.
//

#ifndef DA_PROJECT_FIFO_H
#define DA_PROJECT_FIFO_H

#include "PerfectLinks.h"

class Fifo : public PerfectLinks {

public:
    Fifo(vector<process*> & processes, int curr_id, int m);
    ~Fifo() override;

    int send(int seq, int dest, int sender);
    void rcv(Message** message);

public:
    vector<int> next;
    set<vector<int>> unorderedMessage;
};

#endif //DA_PROJECT_FIFO_H
