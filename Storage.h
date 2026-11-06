#ifndef STORAGE_H
#define STORAGE_H

#include "IMethodOperations.h"
#include <vector>
#include <stdexcept>
#include <cstdint>

class Memory : public IMethodOperations {
private:
    std::vector<uint8_t> CellMem;
    void checkAddress(int address);

public:
    Memory(int size = 256);
    void write(int address, uint8_t value) override;
    uint8_t read(int address) override;
    void clear() override;
};

class Register : public IMethodOperations {
private:
    std::vector<uint8_t> CellReg;

public:
    Register(int size = 16);
    uint8_t read(int address) override;
    void write(int address, uint8_t value) override;
    void clear() override;
};

#endif