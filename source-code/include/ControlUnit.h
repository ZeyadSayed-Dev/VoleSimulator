#ifndef CONTROLUNIT_H
#define CONTROLUNIT_H

#include <cstdint>
#include <string>

class Memory;
class Instruction;
class InstructionFactory;
class MachineContext;

class ControlUnit
{
private:
    int pc;
    int ir;

    int decodedOperands[3];

    Memory& memory;
    InstructionFactory& instructionFactory;
    MachineContext& context;

public:
    ControlUnit(
        Memory& memory,
        InstructionFactory& instructionFactory,
        MachineContext& context
    );

    void fetch();

    bool validate(uint16_t rawInstruction) const;

    Instruction* decode(uint16_t rawInstruction);

    void execute(Instruction* instruction);

    void updatePC(int address);

    int getPC() const;
    void setPC(int address);

    int getIR() const;
    void setIR(int value);

    int* getDecodedOperands();
};

#endif
