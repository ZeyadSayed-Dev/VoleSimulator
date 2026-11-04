#ifndef INSTRUCTIONFACTORY_HPP
#define INSTRUCTIONFACTORY_HPP

#include <string>
#include "Instruction.hpp"
#include "Instructions.hpp"
using namespace std;
class InstructionFactory
{
public:
    Instruction *getInstruction(const string &code)
    {
        if (code == "1")
        {
            return new LoadFromMemory();
        }
        else if (code == "2")
        {
            return new LoadImmediate();
        }
        else if (code == "3")
        {
            return new Store();
        }
        else if (code == "4")
        {
            return new Move();
        }
        else if (code == "5")
        {
            return new AddInteger();
        }
        else if (code == "6")
        {
            return new AddFloatingPoint();
        }
        else if (code == "7")
        {
            return new BitwiseOR();
        }
        else if (code == "8")
        {
            return new BitwiseAND();
        }
        else if (code == "9")
        {
            return new BitwiseXOR();
        }
        else if (code == "A" || opcode == "a")
        {
            return new RotateRight();
        }
        else if (code == "B" || opcode == "b")
        {
            return new Jump();
        }
        else if (code == "C" || opcode == "c")
        {
            return new Halt();
        }
        return nullptr;
    }
};

#endif
