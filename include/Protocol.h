//
// Created by rudra on 01.10.19.
//

#ifndef PROJECT_PROTOCOL_H
#define PROJECT_PROTOCOL_H

#include <string>

using namespace std;

class Protocol {

protected:
    int                 m_socket;
    int                 m_port;
    std::string         m_addr;
    struct addrinfo *   m_addrinfo;

public:

    Protocol(const std::string& addr, int port);
    int                 get_socket() const {return m_socket;}
    int                 get_port() const {return m_port;}
    std::string         get_addr() const {return m_addr;}


    virtual int send(const char * msg, size_t size) = 0;
    virtual int rcv(char * msg, size_t size) = 0;

};

class UDP: public Protocol {

public:
    UDP(const std::string& addr, int port);

    int send(const char * msg, size_t size);
    int rcv(char * msg, size_t size);

};


#endif //PROJECT_PROTOCOL_H
