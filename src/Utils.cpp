#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <iterator>
#include <iostream>

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
    std::string ip = "127.0.0.1";
    int port = 0;
    std::vector<int> affectedProcess;
    struct addinfo * addinfo;
};

/*
 * Convert string to integer
 *
 * @param: stringToConvert = the string to convert to integer
 * @param: errorMessage = the error massage if it fails
 * @param: readFile = the file to close if it fails
 */
int stringToInt(std::string stringToConvert, std::string errorMessage, std::ifstream& readFile) {
    int outputInt;
    std::istringstream iss(stringToConvert);
    iss >> outputInt;
    if (iss.fail()) {
        readFile.close();
        throw errorMessage;
    }
    return outputInt;
}

/*
 * Fonction that parse a given membership text file into process structure
 *
 * @param: fileToParse = string name of the file to parse
 * @return: vector of pointers of process
 */
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

            countIp = stringToInt(countString, "##### error when reading the indexes of the entries IP and Port of the Membership file when parsing (1) #####", file);

            std::istringstream iss(line.erase(0, pos + 1));
            std::vector<std::string> parsedLine((std::istream_iterator<std::string>(iss)),
                                                std::istream_iterator<std::string>());

            processVector.push_back(new(process));
            processVector[countIp-1]->id = countIp;
            processVector[countIp-1]->ip = parsedLine[0];

            int port = stringToInt(parsedLine[1], "##### error when reading the indexes of the entries IP and Port of the Membership file when parsing (2) #####", file);
            processVector[countIp-1]->port = port;
        }

        // Parsing Process Affectation

        int countBroad = 0;
        std::vector<std::vector<int>> processAffectation(nbProcess);
        while (countBroad < nbProcess) {
            std::getline(file, line);
            std::size_t pos = line.find(delimiter);
            std::string countString = line.substr(0, pos);

            countBroad = stringToInt(countString, "error when reading the indexes of the entries IP and Port of the Membership file when parsing (2)", file);

            std::istringstream issProcess(line.erase(0, pos + 1));
            int intId;
            std::vector<int> affect;
            std::string token;
            while (std::getline(issProcess, token, ' ')) {
                intId = stringToInt(token, "##### error when reading the indexes of the entries IP and Port of the Membership file when parsing (3) #####", file);
                processVector[intId-1]->affectedProcess.push_back(countBroad);
            }
        }
        file.close();
    } else {
        throw std::string("##### error when trying to open the Membership file for parsing #####");
    }
    return processVector;
}

/*
 * Pretty print of a process structure
 *
 * @param: process = a process pointer
 */
void print(process * process){
    std::cout << "ID : " << process->id;
    std::cout << "\nIP : " + process->ip;
    std::cout << "\nPort : " << process->port;
    std::cout << "\nAffected Process : ";
    for (int i = 0; i < (process->affectedProcess).size(); i++) {
        std::cout << (process->affectedProcess).at(i) << "  ";
    }
    std::cout << "\nSocket : " << process->socket << "\n\n\n";
}

//Test parser with exMembership.txt
/*
int main() {
    std::vector<process*> testProcess = parser("exMembership.txt");
    for (int i = 0; i < testProcess.size(); i++) {
        print(testProcess[i]);
    }
    return 0;
}
*/