#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <ctime>
#include <string>
#include <mutex>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include "Protocol.h"
#include "PerfectLinks.h"
#include <unistd.h>
#include "Urb.h"
#include "Fifo.h"
#include "Utils.h"


#define FILENAME "exMembership.txt"
#define MAXCHARS 255
#define bufferSize 50

static int wait_for_start = 1;
static bool finish = false;

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
    finish = true;
}

void *rcv(void * arg) {

    auto *prot = (Protocol *) arg;
    int sock = prot->m_procs[prot->curr_proc]->socket;
    cout << "Start receiving on socket : " << sock << endl;

    // reset buffer for receiving
    while(!prot->end) {
        Message *rm = nullptr;
        prot->rcv(&rm);

        delete rm;
    }

    return arg;

}

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
   while(wait_for_start && !finish) {
        struct timespec sleep_time;
        sleep_time.tv_sec = 0;
        sleep_time.tv_nsec = 1000;
        nanosleep(&sleep_time, NULL);
    }

   if(!finish) {
       //start thread for sending
       //broadcast messages
       printf("Start sending.\n");
       prot->startSending();
   }
    //wait until stopped
    while(!finish) {
        struct timespec sleep_time;
        sleep_time.tv_sec = 1;
        sleep_time.tv_nsec = 0;
        nanosleep(&sleep_time, NULL);
    }

    cout << "Cleanup" << endl;
    prot->finish();
    pthread_join(t1, NULL);
    delete prot;
    return 0;
}