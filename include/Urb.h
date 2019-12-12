//
// Created by pgabioud on 14/11/2019.
//

#ifndef DA_PROJECT_URB_H
#define DA_PROJECT_URB_H

#include "PerfectLinks.h"

/*
 * Universal Reliable Broadcast module
 */
class Urb : public PerfectLinks {

public:
    Urb(vector<process*> & processes, int curr_id, int m);
    ~Urb() override;

    int send(int seq, int dest, int sender, string vc = "");
    void rcv (Message **message);

    /*
     * Check if given message can be deliver
     *
     * @param: key = message that must be check for delivering
     */
    bool canDeliver(pair<string, unsigned> key);

public:
    // ack messages
    unordered_map<pair<string, int>, unordered_set<int>, hash_pair > ack;

    //delivered messages
    unordered_set<pair<string, int>, hash_pair> delivered;
};

#endif //DA_PROJECT_URB_H
