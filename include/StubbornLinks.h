//
// Created by pgabioud on 11/11/2019.
//

#ifndef DA_PROJECT_STUBBORNLINKS_H
#define DA_PROJECT_STUBBORNLINKS_H

#include "UDP.h"

using namespace std;


class StubbornLinks : public UDP {

public:
    StubbornLinks(vector<process*> & processes, int curr_id,int m);
    ~StubbornLinks() override;

    int send(int seq, int dest, int sender);
    void rcv(Message **message);

    mutex slmtx;
};


#endif //DA_PROJECT_STUBBORNLINKS_H
