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
#include "Protocol.h"
#include "UDP.h"
#include <mutex>

#define ACK "ack"
#define TIMEOUT_MS 2


UDP::UDP(vector<process *> &processes, int curr_id, int m)
:Protocol(processes, curr_id, m)
{
}

UDP::~UDP()
{
}

int UDP::send(int seq, int dest, int sender, string vc) {
    sndmtx.lock();
    process *p = m_procs[dest];

    //cout << "UDP send vc : " << vc << endl;
    string payload = to_string(seq) + " " + to_string(sender) + " " + vc+  "\0";
    //cout << "UDP send to : [" << dest + 1 <<"] : [" << seq << " " << sender + 1 << "]" << endl;

    int er = sendto(m_procs[curr_proc]->socket, payload.c_str(), payload.size()+1, 0, (const sockaddr*)(p->addrinfo), sizeof(*p->addrinfo));
    if(er < 0) {
        cerr << "Error sending message : " << payload << " to p" << dest + 1 << endl;
    }
    sndmtx.unlock();
    return er;
}

int UDP::sendAck(int seq, int dest, int sender) {
    process *p = m_procs[dest];

    string payload ="ack " + to_string(seq) + " " + to_string(sender) +  "\0";

    int er = sendto(m_procs[curr_proc]->socket, payload.c_str(), payload.size()+ 1, 0, (const sockaddr*)(p->addrinfo), sizeof(*p->addrinfo));

    if(er < 0) {
        cerr << "Error sending message : " << payload << " to p" << dest + 1 << endl;
    }

    return er;

}

void UDP::rcv(Message **m) {
    int sockfd = m_procs[curr_proc]->socket;
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len;
    peer_addr_len = sizeof(peer_addr);
    char host[NI_MAXHOST], service[NI_MAXSERV];

    char msg_buf[255];
    bzero(msg_buf, 0);
    size_t len = 255;

    int er = recvfrom(sockfd, msg_buf, len, 0, (struct sockaddr *) &peer_addr, &peer_addr_len);
    if(er <= 0) {
        (*m) = nullptr;
        return;
    }

    int s = getnameinfo((struct sockaddr *) &peer_addr, peer_addr_len, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
    int port =  ntohs(peer_addr.sin_port) ;

    if (s == 0) {
        int idSource = -1;

        for(auto& p:m_procs) {
            if(p->port == port) {
                idSource = p->id;
            }
        }

        string payload = string(msg_buf);
        auto tokens = split(payload,' ');
        int type = -1;
        int os = -1;
        int seq = -1;
        string sourceVC ="";

        if (payload.find(ACK) != std::string::npos) {
            // message is ack message
            type = 1;
            os = stringToInt(tokens[2]);
            seq = stringToInt(tokens[1]);

        } else {
            type = 0;
            os = stringToInt(tokens[1]);
            seq = stringToInt(tokens[0]);
            sourceVC = tokens[2];
        }

        //cout << "UDP receive from : [" << idSource + 1 <<"] : [" << seq << " " << os + 1 << "]" << endl;
        *m=new Message(idSource,curr_proc, type, os, seq,"", sourceVC);

        (*m)->discard = false;
        //deliver((*m)->seqNum, (*m)->os);
    } else {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
        (*m) = nullptr;
    }
    bzero(msg_buf,strlen(msg_buf));

}
