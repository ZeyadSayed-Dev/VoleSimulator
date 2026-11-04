#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include "MachineContext.hpp"

class Instruction
{
public:
    virtual ~Instruction() = default;
    virtual void execute(MachineContext &context) = 0;
};
#endif
