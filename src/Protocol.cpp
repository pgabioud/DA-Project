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

Protocol::Protocol(const std::string &_addr, int _port)
:m_port(_port), m_addr(_addr)
{
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
    m_socket = socket(m_addrinfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
    if(m_socket == -1)
    {
        freeaddrinfo(m_addrinfo);
        throw runtime_error(("could not create socket for: \"" + m_addr + ":" + decimal_port + "\"").c_str());
    }
}

UDP::UDP(const std::string &_addr, int _port)
:Protocol(_addr, _port)
{}

int UDP::send(const char * msg, size_t size) {
    return sendto(m_socket, msg, size, 0, m_addrinfo->ai_addr, m_addrinfo->ai_addrlen);
}

int UDP::rcv(char *msg, size_t size) {
    int er = recv(m_socket, msg, size, 0);
    if(er < 0) {
        cerr << "Error" << endl;
        return -1;
    }

    return 0;
}