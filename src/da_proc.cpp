#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <ctime>
#include <iostream>
#include <Protocol.h>

#include "Utils.h"

#define FILENAME "exMembership.txt"


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

int main(int argc, char** argv) {

    //set signal handlers
    signal(SIGUSR2, start);
    signal(SIGTERM, stop);
    signal(SIGINT, stop);


    printf("Initializing.\n");

    //parse arguments, including membership
    //string filename = string(argv[2]);
    //int curr_id = atoi(argv[1]);
    int curr_id = 3;
    vector<process*> mProcs = parser(FILENAME);
    //initialize application

    UDP prot(mProcs, curr_id);
    for(auto p : mProcs) {
        cout << *p << endl;
    }

    //start listening for incoming UDP packets

    //start thread for listening
    //start thread for sending

/*
    //wait until start signal
    while(wait_for_start) {
        struct timespec sleep_time;
        sleep_time.tv_sec = 0;
        sleep_time.tv_nsec = 1000;
        nanosleep(&sleep_time, NULL);
    }


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