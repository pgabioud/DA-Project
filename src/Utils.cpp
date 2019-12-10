#include <fstream>
#include <sstream>
#include <cstring>
#include <iterator>
#include <iostream>
#include <string>
#include <utility>

#include "Utils.h"

Message::Message(int sid, int did, int type, int os, int seqNum, string val, string stringVC) {
    this->sid = sid;
    this->did = did;
    this->type = type;
    string payload_;
    if(type == 1) {
        payload_ = "ack ";
    }
    if(type == 2) {
        payload_ = move(val);
    } else {
        payload_ += to_string(seqNum) + " "+ to_string(os);
    }

    this->payload = std::move(payload_);
    this->size = this->payload.size();
    this->os = os;
    this->seqNum = seqNum;
    this->strSourceVC = move(stringVC);
}


int stringToInt(string stringToConvert) {
    int outputInt;
    istringstream iss(stringToConvert);
    iss >> outputInt;
    if (iss.fail()) {
        return -666;
    }
    return outputInt;
}

vector<process*> parser(string fileToParse) {
    ifstream file(fileToParse);                    //normally Membership
    cout << fileToParse << endl;
    vector<process*> processVector;

    //Get number of Process

    if (file) {
        int nbProcess;
        file >> nbProcess;

        //Parsing IP and Ports

        string line;
        int countIp = 0;
        string delimiter = " ";
        getline(file, line);   //skip a line

        while (countIp < nbProcess) {
            getline(file, line);
            size_t pos = line.find(delimiter);
            string countString = line.substr(0, pos);
            countIp = stringToInt(countString);
            if (countIp == -666) {
                file.close();
                throw string("##### error when reading the indexes of the entries IP and Port of the Membership file when parsing (1) #####");
            }
            istringstream iss(line.erase(0, pos + 1));
            vector<string> parsedLine((istream_iterator<string>(iss)),
                                                istream_iterator<string>());

            processVector.push_back(new(process));
            processVector[countIp-1]->id = countIp - 1;
            processVector[countIp-1]->ip = parsedLine[0];

            int port = stringToInt(parsedLine[1]);
            if (port == -666) {
                file.close();
                throw string("##### error when reading the indexes of the entries IP and Port of the Membership file when parsing (2) #####");
            }
            processVector[countIp-1]->port = port;
        }

        // Parsing Process Affectation
/*
        int countBroad = 0;
        vector<vector<int>> processAffectation(nbProcess);
        while (countBroad < nbProcess) {
            getline(file, line);
            size_t pos = line.find(delimiter);
            string countString = line.substr(0, pos);

            countBroad = stringToInt(countString);
            if (countBroad == -666) {
                file.close();
                throw string("error when reading the indexes of the entries IP and Port of the Membership file when parsing (2)");
            }

            istringstream issProcess(line.erase(0, pos + 1));
            int intId;
            vector<int> affect;
            string token;
            while (getline(issProcess, token, ' ')) {
                intId = stringToInt(token);
                if (intId == -666) {
                    file.close();
                    throw string("##### error when reading the indexes of the entries IP and Port of the Membership file when parsing (3) #####");
                }
                processVector[intId-1]->affectedProcess.push_back(countBroad);
            }
        }
*/
        file.close();
    } else {
        throw string("##### error when trying to open the Membership file for parsing #####");
    }
    return processVector;
}

void print(process * process){
    cout << "ID : " << process->id;
    cout << "\nIP : " + process->ip;
    cout << "\nPort : " << process->port;
    cout << "\nAffected Process : ";
    for (unsigned i = 0; i < (process->affectedProcess).size(); i++) {
        cout << (process->affectedProcess).at(i) << "  ";
    }
    cout << "\nSocket : " << process->socket << "\n\n\n";
}

void writeLogs(string logFileName, vector<vector<string>> * logs) {
    ofstream logFile;
    logFile.open(logFileName, ios::out | ios::app);
    if (logFile.is_open()) {
        vector< vector<string> >::iterator iterVector;
        vector<string>::iterator iterString;
        for (iterVector = logs->begin(); iterVector != logs->end(); iterVector++) {
            for (iterString = iterVector->begin(); iterString != iterVector->end(); iterString++) {
                logFile << *iterString << " ";
            }
            logFile << "\n";
        }

        logFile.close();
    } else {
        throw string("##### error when trying to open the log file when wr #####");
    }
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

string vectorClockToString(vector<int> *vectorClock)
{
    string vc = "";
    for(auto const& vec: *vectorClock) {
        vc += to_string(vec) + "#";
    }
    return vc.substr(0, vc.size()-1);
}

void stringToVectorClock(const std::string& vcPayload, vector<int> *vectorClock) {
    int i = 0;
    for (auto const &token: split(vcPayload, '#')) {
        (*vectorClock)[i] = stringToInt(token);
        i += 1;
    }
}