//
// Created by pgabioud on 11/11/2019.
//

#ifndef DA_PROJECT_PERFECTLINKS_H
#define DA_PROJECT_PERFECTLINKS_H

#include "StubbornLinks.h"

using namespace std;

class PerfectLinks : public StubbornLinks {

public:
    PerfectLinks(vector<process*> & processes, int curr_id, int m);
    ~PerfectLinks() override;

    int send(int seq, int dest, int sender);
    void rcv(Message **message);

};


#endif //DA_PROJECT_PERFECTLINKS_H
