//
// Created by pgabioud on 11/11/2019.
//
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <sstream>
#include <netdb.h>
#include <cstring>
#include <algorithm>
#include <fstream>
#include "Urb.h"
#include "Fifo.h"
#include <mutex>


Fifo::Fifo(vector<process *> &procs, int id, int m)
:Urb(procs, id, m)
{
    next.resize(num_procs, 1);
}

Fifo::~Fifo()
{
    next.clear();
    for(auto e : unorderedMessage) {
        e.clear();
    }
    unorderedMessage.clear();
}

int Fifo::send(int seq, int dest, int sender) {
    return Urb::send(seq, dest, sender);
}

void Fifo::rcv(Message **m) {
    Urb::rcv(m);
    if((*m)== nullptr || (*m)->discard) {
        return;
    }

    vector<int> seqOs = {(*m)->seqNum, (*m)->os};

    if(seqOs[0] == 4) {
        cout << "4"  << endl;
    }
    unorderedMessage.insert(seqOs);
    bool iterateAgain;
    vector<vector<int>> msgToDelete;

    do {
        iterateAgain = false;
        set<vector<int>>::iterator it;
        for (it = unorderedMessage.begin(); it != unorderedMessage.end(); it++) {
            int counter = next[(*it)[1]];
            if (counter == (*it)[0]) {
                iterateAgain = true;
                next[(*it)[1]] = counter + 1;
                deliver((*it)[0], (*it)[1]);
                msgToDelete.push_back((*it));
            }
        }
    } while (iterateAgain);

    for(auto& elem: msgToDelete) {
        auto deliveredMsg = find(unorderedMessage.begin(), unorderedMessage.end(), elem);
        unorderedMessage.erase(deliveredMsg);
    }
    return;
}
