#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <ctime>
#include <iostream>
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
    auto* prot = (Protocol*) arg;
    prot->send("Hello!", 7, prot->m_procs[1]);
    cout << "Message sent" << endl;

}

void *rcv(void * arg) {
    auto* prot = (Protocol*) arg;

    // reset buffer for receiving
    memset(prot->rcv_buffer, 0, MAXCHARS + 1);
    int rcv = 0;
    while(rcv == 0) {
        rcv = prot->rcv(prot->rcv_buffer, MAXCHARS, prot->m_procs[prot->curr_proc]);
    }

    cout << "Received :" << prot->rcv_buffer << endl;

}

int main(int argc, char** argv) {

    //set signal handlers
    signal(SIGUSR2, start);
    signal(SIGTERM, stop);
    signal(SIGINT, stop);


    printf("Initializing.\n");

    //parse arguments, including membership
    //string filename = string(argv[2]);
    //int curr_id = atoi(argv[1]);
    int curr_id = 0;

    vector<process*> mProcs = parser(FILENAME);
    //initialize application

    UDP prot(mProcs, curr_id);
    for(auto p : mProcs) {
        cout << *p << endl;
    }

    //start listening for incoming UDP packets
    pthread_t t1, t2;
    int i = 1;
    int j = 2;

    /* Create 2 threads t1 and t2 with default attributes which will execute
    function "thread_func()" in their own contexts with specified arguments. */
    pthread_create(&t1, NULL, &thread_func, &i);
    pthread_create(&t2, NULL, &thread_func, &j);

    /* This makes the main thread wait on the death of t1 and t2. */
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("In main thread\n");


    //wait until start signal
    while(wait_for_start) {
        struct timespec sleep_time;
        sleep_time.tv_sec = 0;
        sleep_time.tv_nsec = 1000;
        nanosleep(&sleep_time, NULL);
    }


    //start thread for listening

    //start thread for sending


/*

    //broadcast messages
    printf("Broadcasting messages.\n");


    //wait until stopped
    while(1) {
        struct timespec sleep_time;
        sleep_time.tv_sec = 1;
        sleep_time.tv_nsec = 0;
        nanosleep(&sleep_time, NULL);
    }

    */
return 0;
}