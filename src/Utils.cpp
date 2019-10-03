#include <fstream>
#include <sstream>
#include <cstring>
#include <iterator>
#include <iostream>

#include "Utils.h"

int stringToInt(string stringToConvert, string errorMessage, ifstream& readFile) {
    int outputInt;
    istringstream iss(stringToConvert);
    iss >> outputInt;
    if (iss.fail()) {
        readFile.close();
        throw errorMessage;
    }
    return outputInt;
}

vector<process*> parser(string fileToParse) {
    ifstream file(fileToParse);                    //normally Membership
    vector<process*> processVector;

    //Get number of Process

    if (file.is_open()) {
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

            countIp = stringToInt(countString, "##### error when reading the indexes of the entries IP and Port of the Membership file when parsing (1) #####", file);

            istringstream iss(line.erase(0, pos + 1));
            vector<string> parsedLine((istream_iterator<string>(iss)),
                                                istream_iterator<string>());

            processVector.push_back(new(process));
            processVector[countIp-1]->id = countIp;
            processVector[countIp-1]->ip = parsedLine[0];

            int port = stringToInt(parsedLine[1], "##### error when reading the indexes of the entries IP and Port of the Membership file when parsing (2) #####", file);
            processVector[countIp-1]->port = port;
        }

        // Parsing Process Affectation

        int countBroad = 0;
        vector<vector<int>> processAffectation(nbProcess);
        while (countBroad < nbProcess) {
            getline(file, line);
            size_t pos = line.find(delimiter);
            string countString = line.substr(0, pos);

            countBroad = stringToInt(countString, "error when reading the indexes of the entries IP and Port of the Membership file when parsing (2)", file);

            istringstream issProcess(line.erase(0, pos + 1));
            int intId;
            vector<int> affect;
            string token;
            while (getline(issProcess, token, ' ')) {
                intId = stringToInt(token, "##### error when reading the indexes of the entries IP and Port of the Membership file when parsing (3) #####", file);
                processVector[intId-1]->affectedProcess.push_back(countBroad);
            }
        }
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
    for (int i = 0; i < (process->affectedProcess).size(); i++) {
        cout << (process->affectedProcess).at(i) << "  ";
    }
    cout << "\nSocket : " << process->socket << "\n\n\n";
}

//Test parser with exMembership.txt
/*
int main() {
    vector<process*> testProcess = parser("exMembership.txt");
    for (int i = 0; i < testProcess.size(); i++) {
        print(testProcess[i]);
    }
    return 0;
}
*/