//
// Created by pgabioud on 11/11/2019.
//

#ifndef DA_PROJECT_UDP_H
#define DA_PROJECT_UDP_H

#include "Protocol.h"

using namespace std;

/*
 * UDP class
 */
class UDP: public Protocol {

public:
    UDP(vector<process*> & processes, int curr_id,int m);
    virtual ~UDP();

    int send(int seq, int dest, int sender, string vc = 0);
    void rcv(Message **message);

    /*
     * send a acknowledgment message
     *
     * @param: seq = sequence number
     * @param: dest = destination id of message
     * @param: sender = id of sender of message
     */
    int sendAck(int seq, int dest, int sender);

    mutex rcvmtx;
    mutex sndmtx;
};
#endif //DA_PROJECT_UDP_H
