#include "Storage.h"

void Memory::checkAddress(int address) {
    if (address < 0 || address >= CellMem.size()) {
        throw std::out_of_range("Address is out of range");
    }
}

Memory::Memory(int size) : CellMem(size) {}

void Memory::write(int address, uint8_t value) {
    checkAddress(address);
    CellMem[address] = value;
}

uint8_t Memory::read(int address) {
    checkAddress(address);
    return CellMem[address];
}

void Memory::clear() {
    for (auto& cell : CellMem) {
        cell = 0;
    }
}

Register::Register(int size) : CellReg(size, 0) {}

uint8_t Register::read(int address) {
    if (address < 0 || address >= CellReg.size()) {
        throw std::out_of_range("Address is out of range");
    }
    if (address == 0)
        return 0;
    return CellReg[address];
}

void Register::write(int address, uint8_t value) {
    if (address < 0 || address >= CellReg.size()) {
        throw std::out_of_range("Address is out of range");
    }
    if (address != 0)
        CellReg[address] = value;
}

void Register::clear() {
    for (auto& cell : CellReg) {
        cell = 0;
    }
}