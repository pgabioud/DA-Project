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
    virtual ~PerfectLinks() override;

    int send(int seq, int dest, int sender, string vc = "");
    void rcv(Message **message);

    //Perfect links no duplication container
    vector<unordered_set<pair<int,int>, hash_pair>> pl_delivered;
};

#endif //DA_PROJECT_PERFECTLINKS_H
