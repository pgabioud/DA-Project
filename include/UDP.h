//
// Created by pgabioud on 11/11/2019.
//

#ifndef DA_PROJECT_UDP_H
#define DA_PROJECT_UDP_H

#include "Protocol.h"

using namespace std;

class UDP: public Protocol {

public:
    UDP(vector<process*> & processes, int curr_id,int m);
    ~UDP() override;

    int send(int seq, int dest, int sender);
    void rcv(Message **message);

    int sendAck(int seq, int dest, int sender);

};
#endif //DA_PROJECT_UDP_H
