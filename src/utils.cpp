#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <iterator>

std::vector< std::vector<std::string>> parser(std::string fileToParse) {    //usually Membership
    //file name parameter
    std::ifstream file(fileToParse);
    std::vector< std::vector<std::string>> parsedFile;

    if (file.is_open()){
        int nbProcess;
        file >> nbProcess;
        std::vector< std::vector<std::string>> definedParsedFile(nbProcess, std::vector<std::string>(2));
        parsedFile = definedParsedFile;

        std::cout << nbProcess;
        std::cout << "\n";

        std::string line;
        int count = 0;
        std::string delimiter = " ";
        std::getline(file, line);   //skip a line

        while( std::getline(file, line) && (count < nbProcess) ) {
            std::size_t pos = line.find(delimiter);
            std::string countString = line.substr(0, pos);

            std::istringstream issCount (countString);
            issCount >> count;
            if (issCount.fail()) {
                file.close();
                throw std::string("Error when reading the indexes of the entries of the Membership file when parsing");
            }
            std::cout << count;
            std::cout << ":  ";

            std::istringstream iss (line.erase(0, pos+1));
            std::vector<std::string> parsedLine((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

            std::cout << parsedLine[0];
            std::cout << "\n";

            parsedFile[count] = parsedLine;
        }
        file.close();
    } else {
        throw std::string("Error when trying to open the Membership file for parsing");
    }
    return parsedFile;
}