
#ifndef UTILITIES_H
#define UTILITIES_H

#include "IMethodOperations.h"
#include <string>
#include <iostream>
#include <vector>
#include <cstdint>
using namespace std;
class OutputAndInput {
    string input;
    string output;

public:
    void write(const string& out);
    string read();
    void clear();
};

class ProgramLoader {
public:
    void loadProgram(IMethodOperations& storage, const vector<uint8_t>& programBytes, int startAddress = 0);
    void clear(IMethodOperations& storage);
    vector<uint8_t> parseProgramText(const string& text) const;
    vector<uint8_t> readProgramFile(const string& fileName) const;
    void validateProgram(const vector<uint8_t>& programBytes, int startAddress) const;
    void loadProgramFile(IMethodOperations& storage, const string& fileName, int startAddress = 0);
};

#endif
