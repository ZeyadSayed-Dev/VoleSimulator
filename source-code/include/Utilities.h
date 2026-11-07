
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
    // Writes an already validated byte program into storage.
    void loadProgram(IMethodOperations& storage, const vector<uint8_t>& programBytes, int startAddress = 0);
    void clear(IMethodOperations& storage);

    // 1. Reads text made of 4-hex-digit instructions.
    // 2. Everything from ';' to the end of a line is a comment and whitespace is ignored.
    // 3. std::invalid_argument when a non-hex character is found, when the number of hex digits is not a multiple of 4, or when there are no instructions.
    vector<uint8_t> parseProgramText(const string& text) const;

    // Reads a file and converts it with parseProgramText().
    vector<uint8_t> readProgramFile(const string& fileName) const;

    // Throws when the program is empty, the start address is not an even address in 00..FE, or the program does not fit in the 256 byte memory.
    void validateProgram(const vector<uint8_t>& programBytes, int startAddress) const;

    // Reads, validates and then writes a file into storage.
    void loadProgramFile(IMethodOperations& storage, const string& fileName, int startAddress = 0);
};

#endif
