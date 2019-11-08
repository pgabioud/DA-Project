#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <ctime>
#include <string>
#include <iostream>
#include <pthread.h>
#include "Protocol.h"
#include "Utils.h"

#define FILENAME "exMembership.txt"
#define MAXCHARS 255

static int wait_for_start = 1;
string log;

static void start(int signum) {
    wait_for_start = 0;
}

static void stop(int signum) {
    //reset signal handlers to default
    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL);

    //immediately stop network packet processing
    printf("Immediately stopping network packet processing.\n");

    //write/flush output file if necessary
    printf("Writing output.\n");


    //exit directly from signal handler
    exit(0);
}

#include <mutex>
mutex mtxm;
void* work(void* arg) {
    process_send_t* tmp = (process_send_t*) arg;
    Urb* prot = tmp->p;
    int did = tmp->did;
    cout << "Created thread for :" << did << endl;
    while(true) {
        // attempt to broadcast
        set<int>::iterator it = prot->bmessages[did].begin();
        for(unsigned long i =0; i< prot->bmessages[did].size(); i++) {

            Message * m = new Message(prot->curr_proc, did, false, prot->curr_proc, *it);
            prot->send(m);
            delete m;
            std::advance(it, 1);
        }

        //clean PL Layer
        for(auto i : prot->sl_delivered[did]) {
            prot->bmessages[did].erase(i);
        }

        //attempt to rebroadcast to corresponding processes
        for(int pid=0;pid< prot->num_procs; pid++) {
            //Rebroadcast all messages seen from process pid to all remaining processes
            for(unsigned long mid=0; mid < prot->proc_rebroadcast_queue[pid].size();mid++) {
                for(int rpid= 0; rpid< prot->num_procs; rpid++) {
                    if(rpid != prot->curr_proc) {
                        Message * m = new Message(prot->curr_proc, rpid, false, pid , mid);
                        prot->send(m);
                        delete m;
                    }
                }
            }
        }
    }

}

void *send(void* arg) {
    PerfectLinks* prot = (PerfectLinks*)arg;

    for(int i=0; i < prot->num_procs; i++) {
        if(i == prot->curr_proc) {
            continue;
        }

        process_send_t* arg  = (process_send_t*)malloc(sizeof(process_send_t));
        pthread_t t;
        arg->p = prot;
        arg->did = i;
        pthread_create(&t, NULL, &work, (void*)arg);
    }

}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
void *rcv(void * arg) {

    auto *prot = (Protocol *) arg;
    int sock = prot->m_procs[prot->curr_proc]->socket;
    cout << "Start receiving on socket : " <<sock << endl;

    // reset buffer for receiving
    while(1) {

        Message* rm = nullptr;
        prot->rcv(&rm);

        if (rm ==nullptr) {
            //discard
            continue;
        }

        if(rm->discard) {
            delete rm;
            continue;
        }

        cout << "Delivering " << *rm << endl;
        // write to log
        vector<string> newLog = {"d", to_string(rm->os + 1), to_string(rm->seqNum)};
        logBuffer.push_back(newLog);
        if (logBuffer.size() <= prot->sizeBuffer) {
            writeLogs(prot->log, &logBuffer);
            logBuffer.clear();
        }

        delete rm;

    }



}
#pragma clang diagnostic pop

int main(int argc, char** argv) {

    //set signal handlers
    signal(SIGUSR2, start);
    signal(SIGTERM, stop);
    signal(SIGINT, stop);

    printf("Initializing.\n");

    //parse arguments, including membership
    string filename = FILENAME;

    int curr_id = 1;
    int m = 1;
    if(argc != 1) {
        curr_id = atoi(argv[1]);
        filename = string(argv[2]);
        m = atoi(argv[3]);
    }
    //initialize application

    vector<process*> mProcs = parser(filename);
    auto *prot = new Urb(mProcs, curr_id - 1, m);

    cout << "Protocol initiated" << endl;

    pthread_t t1, t2;
    //start listening for incoming UDP packets
    pthread_create(&t1, NULL, &rcv, (void *) prot);

    //wait until start signal
   while(wait_for_start) {
        struct timespec sleep_time;
        sleep_time.tv_sec = 0;
        sleep_time.tv_nsec = 1000;
        nanosleep(&sleep_time, NULL);
    }

   //start thread for sending
    printf("Broadcasting messages.\n");

    cout << "Start sending" << endl;

    pthread_create(&t1, NULL, &send, (void *) prot);

    //broadcast messages

    //wait until stopped
    while(1) {
        struct timespec sleep_time;
        sleep_time.tv_sec = 1;
        sleep_time.tv_nsec = 0;
        nanosleep(&sleep_time, NULL);
    }

    pthread_exit(NULL);
    return 0;
}