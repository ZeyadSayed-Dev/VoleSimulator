#include "ALU.h"

#include <cmath>
#include <limits>

// 1. Integer Addition (Opcode 5)
uint8_t ALU::addInteger(uint8_t a, uint8_t b) const {
    return static_cast<uint8_t>(
        static_cast<unsigned int>(a) +
        static_cast<unsigned int>(b)
    );
}

// 2. Floating Addition (Opcode 6)
uint8_t ALU::addFloatingPoint(uint8_t a, uint8_t b) const {

    // Decode operands
    int signA = (a >> 7) & 1;
    int exponentA = (a >> 4) & 0x07;
    int mantissaA = a & 0x0F;
    
    int signB = (b >> 7) & 1;
    int exponentB = (b >> 4) & 0x07;
    int mantissaB = b & 0x0F;

    // Convert operands into numerical values
    long double valueA =
        std::pow(2.0L, exponentA - 4) *
        (static_cast<long double>(mantissaA) / 16.0L);

    long double valueB =
        std::pow(2.0L, exponentB - 4) *
        (static_cast<long double>(mantissaB) / 16.0L);

    if (signA) valueA = -valueA;
    if (signB) valueB = -valueB;

    // Perform addition
    long double result = valueA + valueB;

    // Handle zero
    if (result == 0.0L) {
        return 0x00;
    }

    // Determine result sign
    int resultSign = result < 0 ? 1 : 0;

    long double magnitude = std::abs(result);

    // Maximum representable positive magnitude:
    // Mantissa = 15, Exponent = 7
    const long double MAX_VALUE =
        15.0L * std::pow(2.0L, 7 - 8);

    // Saturate on overflow
    if (magnitude >= MAX_VALUE) {
        return static_cast<uint8_t>(
            (resultSign << 7) | (7 << 4) | 0x0F
        );
    }

    // Find the closest representable value
    long double bestError =
        std::numeric_limits<long double>::max();

    uint8_t bestResult = 0;

    bool bestNormalized = false;

    for (int exponent = 0; exponent <= 7; exponent++) {

        for (int mantissa = 0; mantissa <= 15; mantissa++) {

            long double candidate =
                mantissa * std::pow(2.0L, exponent - 8);

            long double error =
                std::abs(magnitude - candidate);

            bool normalized = mantissa >= 8;

            if (error < bestError ||
                (error == bestError &&
                 normalized && !bestNormalized)) {

                bestError = error;
                bestNormalized = normalized;

                bestResult = static_cast<uint8_t>(
                    (resultSign << 7) |
                    (exponent << 4) |
                    mantissa
                );
            }
        }
    }

    return bestResult;
}

// 3. Bitwise OR (Opcode 7)
uint8_t ALU::bitwiseOR(uint8_t a, uint8_t b) const {
    return a | b;
}

// 4. Bitwise AND (Opcode 8)
uint8_t ALU::bitwiseAND(uint8_t a, uint8_t b) const {
    return a & b;
}

// 5. Bitwise XOR (Opcode 9)
uint8_t ALU::bitwiseXOR(uint8_t a, uint8_t b) const {
    return a ^ b;
}

// 6. Rotate Right (Opcode A)
uint8_t ALU::rotateRight(uint8_t value, int count) const {

    count %= 8;

    if (count < 0) {
        count += 8;
    }

    if (count == 0) {
        return value;
    }

    return static_cast<uint8_t>(
        (value >> count) |
        (value << (8 - count))
    );
}



