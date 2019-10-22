#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <ctime>
#include <sstream>
#include <string>
#include <iostream>
#include <pthread.h>
#include "Protocol.h"
#include "Utils.h"

#define FILENAME "exMembership.txt"
#define MAXCHARS 255


static int wait_for_start = 1;

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

void *send(void* arg) {
    cout << "Start sending" << endl;
    UDP* prot = (UDP*) arg;
    for(auto p : prot->m_procs) {
        if(p->id!= prot->curr_proc + 1) {
            stringstream ss;
            ss << "Hello! from " << prot->curr_proc << endl;
            cout << "Sending : " << ss.str();
            prot->send("Hi", MAXCHARS, p->id );
            cout << "Message sent to " << p->id <<  endl;
            ss.clear();
        }
    }

}

void *rcv(void * arg) {
    cout << "Start receiving" << endl;

    // reset buffer for receiving
    int rcv = 0;
    UDP * prot = (UDP *)arg;
    char buf[MAXCHARS];
    memset(buf, 0, MAXCHARS);

    rcv = prot->rcv(buf,MAXCHARS);

    cout << "Received :" << *buf << endl;
    cout << "Receiving Done" << endl;
}

int main(int argc, char** argv) {

    //set signal handlers
    signal(SIGUSR1, start);
    signal(SIGTERM, stop);
    signal(SIGINT, stop);


    printf("Initializing.\n");

    //parse arguments, including membership
    string filename = FILENAME;
    //int curr_id = atoi(argv[1]);

    int curr_id = 1;
    if(argc != 1) {
        curr_id = atoi(argv[1]);
        filename = string(argv[2]);
    }
    //initialize application

    vector<process*> mProcs = parser(filename);
    UDP *prot = new UDP(mProcs, curr_id);

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
    pthread_create(&t2, NULL, &send, (void *) prot);

    //broadcast messages

    pthread_join(t2, NULL);

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