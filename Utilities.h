
#ifndef UTILITIES_H
#define UTILITIES_H

#include "IMethodOperations.h"
#include <string>
#include <iostream>
#include <vector>
#include <cstdint>

class OutputAndInput {
    std::string input;
    std::string output;

public:
    void write(const std::string& out);
    std::string read();
    void clear();
};

class ProgramLoader {
public:
    void loadProgram(IMethodOperations& storage, const std::vector<uint8_t>& programBytes, int startAddress = 0);
    void clear(IMethodOperations& storage);
};

#endif