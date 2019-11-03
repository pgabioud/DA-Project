//
// Created by rudra on 03.10.19.
//

#ifndef DA_PROJECT_UTILS_H
#define DA_PROJECT_UTILS_H

#include <vector>
#include <cstring>
using namespace std;

/*
 * Process structure
 *
 * id = the id of the process
 * ip = ip address of the process
 * socket = socket number of the process
 * port = port number of the process
 * affectedProcess = integer vector of all the processes affected to the process
 * addinfo = contains information relative to the UDP connection
 */
struct process{
    int id = 0;
    int socket = -1;
    string ip = "127.0.0.1";
    int port = 0;
    vector<int> affectedProcess;
    struct sockaddr_in * addrinfo;
};

class Message{
public:
    int sid;
    int did;
    string payload;
    size_t size;
    bool ack = false;

public:
    Message(int sid, int did, string payload, size_t size, bool ack);

    bool operator==(Message const& other) const {
        return this->sid == other.sid
            && this->payload == other.payload;
    }
};

inline ostream& operator<<(ostream& os, const Message& m) {
    return os << "Message (sender: " << m.sid << ", deliverer: "<< m.did + 1<< ", payload: " << m.payload
        << ", size: " << m.size << ", ack : "<< m.ack<< ")";
}

inline ostream& operator<<(ostream& os, const process& p) {
    os << "Process (" << p.id << ") - ip: " << p.ip << ", port: " << p.port;

    if(p.socket != -1) {
        os << " , socket : " << p.socket;
    }
}


/*
 * Pretty print of a process structure
 *
 * @param: process = a process pointer
 */
void print(process * process);

/*
 * Convert string to integer
 *
 * @param: stringToConvert = the string to convert to integer
 */
int stringToInt(string stringToConvert) ;

/*
 * Fonction that parse a given membership text file into process structure
 *
 * @param: fileToParse = string name of the file to parse
 * @return: vector of pointers of process
 */
vector<process*> parser(string fileToParse) ;

/*
 * Write the logs into a text file from a given vector of vector of logs
 *
 * @param: logFileName = string name of the log file
 * @param: logs = vector of logs, each in the form of a vector of string
 */
void writeLogs(string logFileName, vector<vector<string>> * logs);


#endif //DA_PROJECT_UTILS_H
