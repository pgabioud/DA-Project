//
// Created by pgabioud on 03/12/2019.
//

#ifndef DA_PROJECT_LOCALIZEDCAUSAL_H
#define DA_PROJECT_LOCALIZEDCAUSAL_H

#include "Urb.h"

class LocalizedCausal : Urb {

public:
    LocalizedCausal(vector<process*> & processes, int curr_id, int m);
    ~LocalizedCausal() override;

    int send(int seq, int dest, int sender);
    void rcv(Message **message);

    //bool canDeliver(pair<string, unsigned> key);
};

#endif //DA_PROJECT_LOCALIZEDCAUSAL_H
