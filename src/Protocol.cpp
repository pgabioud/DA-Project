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
#include <algorithm>
#include <fstream>
#include "Protocol.h"

#define ACK "ack"

void* work(void* arg);


void Protocol::init_socket(process* proc) {
    int port = proc->port;
    string m_addr = proc->ip;

    struct sockaddr_in* addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    int fd;

    if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket");
        exit(1);
    }

    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(m_addr.c_str());
    addr->sin_port = htons(port);

    if(curr_proc == proc->id) {
        if (bind(fd, (struct sockaddr *) addr, sizeof(*addr)) < 0) {
            perror("bind");
            exit(1);

        }
    }

    proc->addrinfo = addr;
    proc->socket = fd;
}

Protocol::Protocol(vector<process*> &processes, int curr_id, int m)
:m_procs(processes), num_procs(processes.size()),curr_proc(curr_id), acks_per_proc(num_procs), numMess(m)

{
    for(auto & p: m_procs) {
            init_socket(p);
    }
    cout << "Sockets inited" << endl;
    for(auto& p:m_procs) {
        cout << *p << endl;
    }

    //init variables
    //init vectors
    seen = set<string>();
    threads = vector<pthread_t>();
    seqNum = 1;
    stringstream ss;
    ss << "da_proc_" << curr_id + 1 << ".out";
    log = ss.str();
    // reset log
    ofstream ofs;
    ofs.open(log, std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    // create thread for working
    int i;
    pthread_t t;
    pthread_create(&t, NULL, &work, (void*)this);

}

Protocol::~Protocol()
{


}

UDP::UDP(vector<process*> &procs, int id, int m)
:Protocol(procs, id, m)
{}

void* send_thread(void* arg) {
    auto* args = (send_args*)arg;

    Protocol* prot = args->prot;
    int er = -1;
    er = prot->send(args->m);
    *((int*)arg) = er;
    return arg;
}

void* work(void* arg) {
    auto* prot = (Protocol*)arg;

    while(true) {
        if(!prot->work_queue.empty()) {

            Message* m = prot->work_queue.front();
            prot->work_queue.pop();

            //create thread for sending
            auto *args = (send_args *) malloc(sizeof(send_args));
            args->prot = prot;
            args->m = m;
            pthread_t t;
            pthread_create(&t, NULL, &send_thread, (void *) args);

            void* status;
            pthread_join(t,&status);

            auto er = *((int*)status);

        }
    }
}

UDP::~UDP()
{}

int Protocol::broadcast() {
    string payload = to_string(seqNum) + " "+ to_string(curr_proc);
    for(auto& p : m_procs) {
        if(p->id!= curr_proc) {
            auto* m = new Message(curr_proc, p->id, false, curr_proc, seqNum);
            work_queue.push(m);
            cout << "Push " << *m << endl;
        }
    }


    seqNum++;
}

int UDP::send(Message *message) {
    auto *p = m_procs[message->did];

    int er = sendto(m_procs[curr_proc]->socket, message->payload.c_str(), message->payload.size(), 0, (const sockaddr*)(p->addrinfo), sizeof(*p->addrinfo));
    if(er < 0) {
        cerr << "Error sending message : " << message << endl;
    }
    return er;
}

vector<string> split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

Message* UDP::rcv(Message *upper_m) {
    int sockfd = m_procs[curr_proc]->socket;
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len;
    peer_addr_len = sizeof(peer_addr);
    char host[NI_MAXHOST], service[NI_MAXSERV];

    char msg_buf[255];
    size_t len = 255;

    int er = recvfrom(sockfd, msg_buf, len, MSG_DONTWAIT, (struct sockaddr *) &peer_addr, &peer_addr_len);

    if(er < 0) {
        auto* m = new Message(-1,-1,false, -1, -1);
        m->discard = true;
        return m;
    }

    Message* m;

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
        //cout << "FL Received : [" << payload << "]" << endl;
        auto tokens = split(payload,' ');
        bool ack = false;
        int os = -1;
        int seq = -1;
        if (payload.find(ACK) != std::string::npos) {
            // message is ack message
            ack = true;
            os = stringToInt(tokens[2]);
            seq = stringToInt(tokens[1]);
        } else {
            os = stringToInt(tokens[1]);
            seq = stringToInt(tokens[0]);
        }

        m = new Message(idSource,curr_proc, ack, os, seq);
    } else {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
        m = new Message(-1,-1,false, -1, -1);
        m->discard = true;
    }

    bzero(msg_buf,strlen(msg_buf));

    return m;
}


//Stubborn Links Modules
StubbornLinks::StubbornLinks(vector<process *> &procs, int id, int m)
:UDP(procs, id, m)
{}

StubbornLinks::~StubbornLinks()
{
}

int StubbornLinks::send(Message *m) {

    // message is never ack so payload is always the seq number
    string pload(m->payload);
    //cout << "Sending : [" << pload << "]" << endl;
    int stry = 1;
    int er = -1;
    while(find(acks_per_proc[m->did].begin(), acks_per_proc[m->did].end(), pload ) == acks_per_proc[m->did].end() and stry < max_try) {
        er = UDP::send(m);
        stry++;
    }

    if(find(acks_per_proc[m->did].begin(), acks_per_proc[m->did].end(), pload ) == acks_per_proc[m->did].end()) {
        work_queue.push(m);
    }

    return er;
    //cout << "Done Sending : [" << pload << "]" << endl;

    //delete ack
    //acks_per_proc[m->did].erase(find(acks_per_proc[m->did].begin(), acks_per_proc[m->did].end(), pload ));

}

Message* StubbornLinks::rcv(Message *m_) {

    Message* m = UDP::rcv(NULL);
    /*if(m->sid == 0 or m->sid == 1 ) {
        cout << "SL Received : [" << m->payload << "]" << endl;
    }
*/
    if(m->discard) {
        //Discard message
        return m;
    }

    if(m->ack) {
        // payload is of format "ack # #"
        string pload = m->payload.substr(4);
        acks_per_proc[m->sid].insert(pload);
        return m;
    } else {
        // send ack
        auto* ackMess = new Message(m->did, m->sid,  true, m->os, m->seqNum);
        UDP::send(ackMess);
        //cout << "SL Sent ack : ["<< ackMess->payload << "]" << endl;
        delete ackMess;
        return m;
    }
}


//Perfect Links Module
PerfectLinks::PerfectLinks(vector<process *> &procs, int id, int m)
:StubbornLinks(procs, id, m),pl_delivered(num_procs)
{}

PerfectLinks::~PerfectLinks()
{
}

int PerfectLinks::send(Message *message) {
    return StubbornLinks::send(message);
}

void *single_send(void* arg) {
    auto* args = (send_args*)arg;
    cout << "Rebroadcasting to : " << args->m->did << endl;
    auto * pl_send = (PerfectLinks*)args->prot;
    pl_send->send(args->m);

}

void *broadcast_to_p(void* arg) {
    auto* args = (send_args*)arg;
    int seq=0;
    Protocol* prot = args->prot;
    int did= args->did;

    for(int i=0; i < prot->numMess; i++) {
        seq++;
        auto* m = new Message(prot->curr_proc,did,false,prot->curr_proc,seq);
        args->prot->send(args->m);
    }


}

Message* PerfectLinks::rcv(Message *message) {

    auto* m = StubbornLinks::rcv(nullptr);

    if(!m->discard) {
        string pload = m->payload;
        auto found = find(pl_delivered[m->sid].begin(), pl_delivered[m->sid].end(), pload );
        if(found == pl_delivered[m->sid].end()) {
            // did not find
            pl_delivered[m->sid].insert(pload);

            auto sfound = find(seen.begin(), seen.end(), pload );
            if(sfound == seen.end()) {
                seen.insert(pload);
                // only rebroadcast if we have not yet seen this message, do not rebroadcast twice
                // do not rebroadcast acks
                if(!m->ack) {
                    for (auto& p : m_procs) {
                        if (p->id != curr_proc) {

                            auto *rm = new Message(curr_proc, p->id, false, m->os, m->seqNum);
                            work_queue.push(rm);
                        }
                    }
                }
            }

        } else {
            //discard
            m->discard = true;
        }
    }

    if(m->ack) {
        m->discard = true;
    }

    return m;
}


Urb::Urb(vector<process *> &procs, int id, int m)
:PerfectLinks(procs, id,m)
{
    vectorClock.resize(num_procs, vector<int>(m, 1));
}

Urb::~Urb()
{
}

int Urb::send(Message *m) {
    for(auto& s : pendingMessage) {
        if(s==m->payload) {
            return PerfectLinks::send(m);
        }
    }

    bool is_delivered =false;
    for(auto& s : urb_delivered) {
        if(s==m->payload) {
            is_delivered = true;
            break;
        }
    }

    if(!is_delivered) {
        pendingMessage.insert(m->payload);
    }
    return PerfectLinks::send(m);


}

Message* Urb::rcv(Message *message) {
    Message* m = PerfectLinks::rcv(nullptr);

    int iseq = m->seqNum - 1;

    if(m->discard) {
        return m;
    }

    //receive message
    cout << "URB receive : [" << m->payload << "]" << endl;
    vectorClock[m->os][iseq] += 1;

    bool is_pending = false;
    double half = num_procs * 0.5;
    bool is_delivered = false;

    for (auto& s:pendingMessage) {
        if (s == m->payload) {
            is_pending = true;
            break;
        }
    };

    if(!is_pending) {
        pendingMessage.insert(m->payload);
        is_pending = true;
    }

    for (auto& s:urb_delivered) {
        if (s == m->payload) {
            is_delivered = true;
            break;
        }
    };

    bool can_deliver = vectorClock[m->os][iseq] > half;

    if(can_deliver and is_pending and !is_delivered) {
        cout << "URB Deliver : "<< m->payload << endl;
        pendingMessage.erase(m->payload);
        urb_delivered.insert(m->payload);
        return m;
    } else {
        m->discard = true;
    }

    return m;
}
