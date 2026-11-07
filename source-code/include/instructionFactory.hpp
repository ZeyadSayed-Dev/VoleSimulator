#ifndef INSTRUCTIONFACTORY_HPP
#define INSTRUCTIONFACTORY_HPP

#include <string>

#include "Instructions.hpp"

class InstructionFactory
{
public:
    Instruction *getInstruction(const std::string &code)
    {
        static LoadFromMemory loadFromMemory;
        static LoadImmediate loadImmediate;
        static Store store;
        static Move move;
        static AddInteger addInteger;
        static AddFloatingPoint addFloatingPoint;
        static BitwiseOR bitwiseOR;
        static BitwiseAND bitwiseAND;
        static BitwiseXOR bitwiseXOR;
        static RotateRight rotateRight;
        static Jump jump;
        static Halt halt;

        if (code == "1")
        {
            return &loadFromMemory;
        }
        if (code == "2")
        {
            return &loadImmediate;
        }
        if (code == "3")
        {
            return &store;
        }
        if (code == "4")
        {
            return &move;
        }
        if (code == "5")
        {
            return &addInteger;
        }
        if (code == "6")
        {
            return &addFloatingPoint;
        }
        if (code == "7")
        {
            return &bitwiseOR;
        }
        if (code == "8")
        {
            return &bitwiseAND;
        }
        if (code == "9")
        {
            return &bitwiseXOR;
        }
        if (code == "A" || code == "a")
        {
            return &rotateRight;
        }
        if (code == "B" || code == "b")
        {
            return &jump;
        }
        if (code == "C" || code == "c")
        {
            return &halt;
        }

        return nullptr;
    }
};

#endif
