#ifndef ALU_H
#define ALU_H

#include <cstdint>

class ALU {
public:

    // Arithmetic Operations
    uint8_t addInteger(uint8_t a, uint8_t b) const;

    uint8_t addFloatingPoint(uint8_t a, uint8_t b) const;

    // Logical Operations
    uint8_t bitwiseAND(uint8_t a, uint8_t b) const;

    uint8_t bitwiseOR(uint8_t a, uint8_t b) const;

    uint8_t bitwiseXOR(uint8_t a, uint8_t b) const;

    // Rotation Operation
    uint8_t rotateRight(uint8_t value, int count) const;

};

#endif

