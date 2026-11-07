#include "VoleMachine.h"

#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
    std::string toHex4(int value)
    {
        if (value < 0)
        {
            return "-----";
        }

        static const char digits[] = "0123456789ABCDEF";

        std::string result;
        for (int shift = 12; shift >= 0; shift -= 4)
        {
            result += digits[(value >> shift) & 0x0F];
        }

        return result;
    }
}

VoleMachine::VoleMachine()
    : memory(256),
      registerFile(16),
      alu(),
      inputOutput(),
      instructionFactory(),
      context(memory, registerFile, alu, inputOutput),
      controlUnit(memory, instructionFactory, context),
      running(false),
      screen()
{
    // ControlUnit and MachineContext reference each other, so the pointer is
    // wired after both objects exist.
    context.setControlUnit(controlUnit);
    reset();
}

void VoleMachine::step()
{
    if (!running)
    {
        return;
    }

    const int instructionAddress = controlUnit.getPC();

    try
    {
        controlUnit.fetch();
    }
    catch (const std::out_of_range &)
    {
        // Reached the end of memory without a HALT instruction.
        running = false;
        return;
    }

    const uint16_t rawInstruction = static_cast<uint16_t>(controlUnit.getIR());

    if (!controlUnit.validate(rawInstruction))
    {
        running = false;
        throw std::runtime_error(
            "Invalid instruction " + toHex4(rawInstruction) +
            " at address " + toHex4(instructionAddress) + ".");
    }

    Instruction *instruction = controlUnit.decode(rawInstruction);
    if (instruction == nullptr)
    {
        running = false;
        throw std::runtime_error(
            "Could not decode instruction " + toHex4(rawInstruction) +
            " at address " + toHex4(instructionAddress) + ".");
    }

    controlUnit.execute(instruction);
    handleScreenWrite(rawInstruction);

    if (controlUnit.getPC() < 0)
    {
        running = false;
    }
}

void VoleMachine::run()
{
    start();

    while (running)
    {
        step();
    }
}

void VoleMachine::start()
{
    if (controlUnit.getPC() < 0 || controlUnit.getPC() >= 256)
    {
        controlUnit.setPC(0);
    }

    running = true;
}

void VoleMachine::loadProgramFile(const std::string &fileName, int startAddress)
{
    // Read and validate first: if anything is wrong, the current machine state
    // is left untouched.
    ProgramLoader loader;
    const std::vector<uint8_t> programBytes = loader.readProgramFile(fileName);
    loader.validateProgram(programBytes, startAddress);

    reset();
    loader.loadProgram(memory, programBytes, startAddress);

    controlUnit.setPC(startAddress);
    controlUnit.setIR(0);
}

void VoleMachine::reset()
{
    memory.clear();
    registerFile.clear();
    inputOutput.clear();

    controlUnit.setPC(0);
    controlUnit.setIR(0);

    running = false;
    screen.clear();
}

void VoleMachine::halt()
{
    running = false;
    controlUnit.setPC(-1);
}

bool VoleMachine::isRunning() const
{
    return running;
}

bool VoleMachine::isHalted() const
{
    return !running;
}

Register &VoleMachine::getRegisterFile()
{
    return registerFile;
}

Memory &VoleMachine::getMemory()
{
    return memory;
}

ALU &VoleMachine::getALU()
{
    return alu;
}

ControlUnit &VoleMachine::getControlUnit()
{
    return controlUnit;
}

OutputAndInput &VoleMachine::getInputOutput()
{
    return inputOutput;
}

const std::string &VoleMachine::getScreen() const
{
    return screen;
}

void VoleMachine::handleScreenWrite(uint16_t rawInstruction)
{
    const uint8_t opcode = static_cast<uint8_t>((rawInstruction >> 12) & 0x0F);

    // Only STORE (opcode 3) can write to memory.
    if (opcode != 0x3)
    {
        return;
    }

    // Opcode 3 is RXY, so the address is the low byte of the instruction.
    const int address = static_cast<int>(rawInstruction & 0x00FF);
    if (address != 0xFF)
    {
        return;
    }

    // The screen displays the numeric value that was stored (for example
    // storing 19 shows "19", and storing 1 shows "1").
    const std::string rendered = std::to_string(static_cast<unsigned int>(memory.read(0xFF)));

    screen += rendered;
    screen += '\n';

    std::cout << rendered << '\n' << std::flush;
}
