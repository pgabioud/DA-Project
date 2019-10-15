//
// Created by rudra on 01.10.19.
//
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <netdb.h>
#include <cstring>
#include "Protocol.h"

void init_socket(process* proc) {
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
    int m_socket = socket(m_addrinfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
    if(m_socket == -1)
    {
        freeaddrinfo(m_addrinfo);
        throw runtime_error(("could not create socket for: \"" + m_addr + ":" + decimal_port + "\"").c_str());
    }

    proc->socket = m_socket;
}

Protocol::Protocol(vector<process*> &processes, int curr_id)
:m_procs(processes), curr_proc(curr_id)
{
    for(auto & p: m_procs) {
        init_socket(p);
    }
}

UDP::UDP(vector<process*> &procs, int id)
:Protocol(procs, id)
{}

int UDP::send(const char * msg, size_t size, process* p) {
    return sendto(p->socket, msg, size, 0, p->addrinfo->ai_addr, p->addrinfo->ai_addrlen);
}

int UDP::rcv(char *msg, size_t size, process* p) {
    int er = recv(p->socket, msg, size, 0);
    if(er < 0) {
        cerr << "Error" << endl;
        return -1;
    }
    return er;
}