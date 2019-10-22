//
// Created by rudra on 01.10.19.
//
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <sstream>
#include <netdb.h>
#include <cstring>
#include "Protocol.h"

#define LOGFILE "da_proc_n.out"
#define SIZE_LOG_BUFFER 10
vector<vector<string>> logBuffer;

void Protocol::init_socket(process* proc) {
    int m_port = proc->port;
    string m_addr = proc->ip;
    struct addrinfo * m_addrinfo = proc->addrinfo;
    char decimal_port[16];
    snprintf(decimal_port, sizeof(decimal_port), "%d", m_port);
    decimal_port[sizeof(decimal_port) / sizeof(decimal_port[0]) - 1] = '\0';
    struct addrinfo ai;
    memset(&ai, 0, sizeof(ai));
    ai.ai_family = AF_UNSPEC;
    ai.ai_socktype = SOCK_DGRAM;
    ai.ai_protocol = IPPROTO_UDP;
    int r(getaddrinfo(m_addr.c_str(), decimal_port, &ai, &m_addrinfo));
    if(r != 0 || m_addrinfo == NULL)
    {
        throw runtime_error(("invalid address or port: \"" + m_addr + ":" + decimal_port + "\"").c_str());
    }

    //create socket
    int m_socket = socket(m_addrinfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
    if(m_socket == -1)
    {
        freeaddrinfo(m_addrinfo);
        throw runtime_error(("could not create socket for: \"" + m_addr + ":" + decimal_port + "\"").c_str());
    }

    // bind socket
    if(proc->id == curr_proc + 1) {
        r = bind(m_socket, m_addrinfo->ai_addr, m_addrinfo->ai_addrlen);
        if(r != 0)
        {
            //freeaddrinfo(m_addrinfo);
            cerr << "Could not bind socket to " << m_addr << " on port :" << m_port << endl;
        }
    }


    proc->socket = m_socket;
    proc->addrinfo = m_addrinfo;
}

Protocol::Protocol(vector<process*> &processes, int curr_id)
:m_procs(processes), curr_proc(curr_id - 1)
{
    for(auto & p: m_procs) {
        init_socket(p);
        cout << *p << endl;
    }

    //init variables
    seqNum = 1;
    stringstream ss;
    ss << "da_proc_" << curr_id << ".out" <<  endl;
    log = ss.str();


}

UDP::UDP(vector<process*> &procs, int id)
:Protocol(procs, id)
{}

int UDP::send(const char * msg, size_t size, int p_id) {
    auto *p = m_procs[p_id - 1];
/////////////////////////
    string seqNumb = to_string(seqNum);
    vector<string> newLog = {"b", seqNumb};
    logBuffer.push_back(newLog);
    if(logBuffer.size() <= SIZE_LOG_BUFFER) {
        writeLogs(log, &logBuffer);
        logBuffer.clear();
    }
    string newMsg = to_string(curr_proc + 1) + " " + seqNumb;

//////////////////////////////////
    cout << "Sending to socket : [" << p->socket << "] ... message : "<< newMsg <<   endl;
    int er = sendto(p->socket, newMsg.c_str(), size, 0, p->addrinfo->ai_addr, p->addrinfo->ai_addrlen);
    if(er < 0) {
        cerr << "Error sending message : " << newMsg << endl;
    }

    seqNum++;

    return er;
}

int UDP::rcv(char *msg, size_t size) {
    int sockfd = m_procs[curr_proc]->socket;
    cout << "Receiving on socket : [" << sockfd <<"]" <<  endl;
    int er = ::recv(sockfd, msg, size, 0);


///////////////////////////////////
    if(string(msg).empty()) {
        return 0;
    }
    string stringMsg(msg);
    size_t pos = stringMsg.find(" ");
    string processId = stringMsg.substr(0, pos);
    string seqNumb = stringMsg.substr(pos+1, stringMsg.length());
    vector<string> newLog = {"d", processId, seqNumb};
    logBuffer.push_back(newLog);
    if(logBuffer.size() <= SIZE_LOG_BUFFER) {
        writeLogs(log, &logBuffer);
        logBuffer.clear();
    }

//////////////////////////////////
    if(er < 0) {
        cerr << "Error receiving" << endl;
        return -1;
    }
    return er;
}