#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <iterator>
#include <iostream>

struct process{
    int id = 0;
    int socket = -1;
    std::string ip = "127.0.0.1";
    int port = 0;
    std::vector<int> affectedProcess;
    struct addinfo * addinfo;
};

std::vector<process*> parser(std::string fileToParse) {
    std::ifstream file(fileToParse);                    //normally Membership
    std::vector<process*> processVector;

    //Get number of Process

    if (file.is_open()) {
        int nbProcess;
        file >> nbProcess;

        //Parsing IP and Ports

        std::string line;
        int countIp = 0;
        std::string delimiter = " ";
        std::getline(file, line);   //skip a line

        while (countIp < nbProcess) {
            std::getline(file, line);
            std::size_t pos = line.find(delimiter);
            std::string countString = line.substr(0, pos);

            std::istringstream issCount(countString);
            issCount >> countIp;
            if (issCount.fail()) {
                file.close();
                throw std::string(
                        "error when reading the indexes of the entries IP and Port of the Membership file when parsing");
            }

            std::istringstream iss(line.erase(0, pos + 1));
            std::vector<std::string> parsedLine((std::istream_iterator<std::string>(iss)),
                                                std::istream_iterator<std::string>());

            processVector.push_back(new(process));
            processVector[countIp-1]->id = countIp;
            processVector[countIp-1]->ip = parsedLine[0];
            std::istringstream issPort(parsedLine[1]);

            int port;
            issPort >> port;
            if (issPort.fail()) {
                file.close();
                throw std::string(
                        "error when reading the indexes of the entries IP and Port of the Membership file when parsing (2)");
            }
            processVector[countIp-1]->port = port;

        }

        // Parsing Process Affectation

        int countBroad = 0;
        std::vector<std::vector<int>> processAffectation(nbProcess);

        while (countBroad < nbProcess) {
            std::getline(file, line);
            std::size_t pos = line.find(delimiter);
            std::string countString = line.substr(0, pos);

            std::istringstream issCount(countString);
            issCount >> countBroad;
            if (issCount.fail()) {
                file.close();
                throw std::string(
                        "error when reading the indexes of the entries IP and Port of the Membership file when parsing (1)");
            }

            std::istringstream issProcess(line.erase(0, pos + 1));
            int intId;
            std::vector<int> affect;
            std::string token;

            while (std::getline(issProcess, token, ' ')) {
                std::istringstream issToken(token);
                issToken >> intId;
                if (issToken.fail()) {
                    file.close();
                    throw std::string(
                            "error when reading the indexes of the entries IP and Port of the Membership file when parsing (2)");
                }
                processVector[intId-1]->affectedProcess.push_back(countBroad);
            }
        }
        file.close();
    } else {
        throw std::string("##### error when trying to open the Membership file for parsing #####");
    }
    return processVector;
}

int main() {
    std::vector<process*> testProcess = parser("exMembership.txt");
    std::cout << testProcess[0]->id;
    std::cout << testProcess[1]->ip + "\n";
    std::cout << testProcess[2]->port;
    std::cout << "\n";
    for (int i = 0; i < (testProcess[1]->affectedProcess).size(); i++) {
        std::cout << (testProcess[1]->affectedProcess).at(i) << ' ';
    }

    return 0;
}