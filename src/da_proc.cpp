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
    prot->broadcast();

}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
void *rcv(void * arg) {
    UDP *prot = (UDP *) arg;
    vector<vector<string>> logBuffer;
    int sock = prot->m_procs[prot->curr_proc]->socket;
    cout << "Start receiving on socket : " <<sock << endl;

    // reset buffer for receiving
    while(1) {
        Message* rcvMessage = prot->rcv(NULL);

        if (rcvMessage->sid == -1) {
            continue;
        }
        // write to log
        vector<string> newLog = {"d", to_string(rcvMessage->sid), rcvMessage->payload};
        logBuffer.push_back(newLog);
        if (logBuffer.size() <= prot->sizeBuffer) {
            writeLogs(prot->log, &logBuffer);
            logBuffer.clear();
        }
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