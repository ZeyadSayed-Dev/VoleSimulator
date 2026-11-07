#ifndef VOLEMACHINE_H
#define VOLEMACHINE_H

#include <cstdint>
#include <string>

#include "ALU.h"
#include "ControlUnit.h"
#include "MachineContext.h"
#include "Storage.h"
#include "Utilities.h"
#include "instructionFactory.hpp"

// Owns every hardware component of the simulated Vole machine and drives the
// fetch -> validate -> decode -> execute cycle.
class VoleMachine
{
public:
    VoleMachine();

    // Executes one fetch/decode/execute cycle.
    void step();

    // Executes until the machine halts.
    void run();

    // Marks the machine as ready to execute (used before run/step).
    void start();

    void reset();
    void halt();

    bool isRunning() const;
    bool isHalted() const;

    Register &getRegisterFile();
    Memory &getMemory();
    ALU &getALU();
    ControlUnit &getControlUnit();
    OutputAndInput &getInputOutput();
    const std::string &getScreen() const;

private:
    // Handles the "write to address FF is writing to the screen" mapping.
    void handleScreenWrite(uint16_t rawInstruction);

    Memory memory;
    Register registerFile;
    ALU alu;
    OutputAndInput inputOutput;
    InstructionFactory instructionFactory;
    MachineContext context;
    ControlUnit controlUnit;

    bool running;
    std::string screen;
};

#endif
