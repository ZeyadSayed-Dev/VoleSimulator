#ifndef MACHINECONTEXT_H
#define MACHINECONTEXT_H

#include "ALU.h"
#include "ControlUnit.h"
#include "Storage.h"
#include "Utilities.h"

// Bundles the machine components that an instruction needs while executing.
// MachineContext does not own any component; it only holds references to them.
class MachineContext
{
private:
    Memory &memory;
    Register &registerFile;
    ALU &alu;
    OutputAndInput &inputOutput;
    ControlUnit *controlUnit;

public:
    MachineContext(
        Memory &memory,
        Register &registerFile,
        ALU &alu,
        OutputAndInput &inputOutput)
        : memory(memory),
          registerFile(registerFile),
          alu(alu),
          inputOutput(inputOutput),
          controlUnit(nullptr)
    {
    }

    // The control unit is set after construction because ControlUnit itself
    // needs a reference to the context (circular dependency).
    void setControlUnit(ControlUnit &controlUnit)
    {
        this->controlUnit = &controlUnit;
    }

    Register &getRegisterFile() const
    {
        return registerFile;
    }

    Memory &getMemory() const
    {
        return memory;
    }

    ALU &getALU() const
    {
        return alu;
    }

    ControlUnit &getControlUnit() const
    {
        return *controlUnit;
    }

    OutputAndInput &getInputOutput() const
    {
        return inputOutput;
    }
};

#endif
