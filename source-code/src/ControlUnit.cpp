#include "ControlUnit.h"

#include "MachineContext.h"
#include "Storage.h"
#include "instructionFactory.hpp"

#include <stdexcept>
#include <string>

// Constructor
ControlUnit::ControlUnit(
    Memory& memory,
    InstructionFactory& instructionFactory,
    MachineContext& context
)
    : pc(0),
      ir(0),
      decodedOperands{0, 0, 0},
      memory(memory),
      instructionFactory(instructionFactory),
      context(context)
{
}

// FETCH
void ControlUnit::fetch()
{
    if (pc < 0 || pc + 1 >= 256)
    {
        throw std::out_of_range("Invalid program counter");
    }

    uint8_t highByte = memory.read(pc);
    uint8_t lowByte = memory.read(pc + 1);

    ir = (static_cast<uint16_t>(highByte) << 8) | lowByte;

    pc += 2;
}

// VALIDATE
bool ControlUnit::validate(uint16_t rawInstruction) const
{
    uint8_t opcode = (rawInstruction >> 12) & 0x0F;
    uint8_t operand1 = (rawInstruction >> 8) & 0x0F;
    uint8_t operand2 = (rawInstruction >> 4) & 0x0F;

    if (opcode < 0x1 || opcode > 0xC)
    {
        return false;
    }

    // MOVE: 40RS
    if (opcode == 0x4 && operand1 != 0)
    {
        return false;
    }

    // ROTATE RIGHT: AR0X
    if (opcode == 0xA && operand2 != 0)
    {
        return false;
    }

    // HALT: C000
    if (opcode == 0xC && rawInstruction != 0xC000)
    {
        return false;
    }

    return true;
}

// DECODE
Instruction* ControlUnit::decode(uint16_t rawInstruction)
{
    if (!validate(rawInstruction))
    {
        return nullptr;
    }

    uint8_t opcode = (rawInstruction >> 12) & 0x0F;

    uint8_t operand1 = (rawInstruction >> 8) & 0x0F;
    uint8_t operand2 = (rawInstruction >> 4) & 0x0F;
    uint8_t operand3 = rawInstruction & 0x0F;

    decodedOperands[0] = 0;
    decodedOperands[1] = 0;
    decodedOperands[2] = 0;

    switch (opcode)
    {
        case 0x1:
        case 0x2:
        case 0x3:
        case 0xB:

            decodedOperands[0] = operand1;
            decodedOperands[1] = (operand2 << 4) | operand3;

            break;

        case 0x4:

            decodedOperands[0] = operand2;
            decodedOperands[1] = operand3;

            break;

        case 0x5:
        case 0x6:
        case 0x7:
        case 0x8:
        case 0x9:

            decodedOperands[0] = operand1;
            decodedOperands[1] = operand2;
            decodedOperands[2] = operand3;

            break;

        case 0xA:

            decodedOperands[0] = operand1;
            decodedOperands[1] = operand3;

            break;

        case 0xC:
            break;

        default:
            return nullptr;
    }

    const char hexDigits[] = "0123456789ABCDEF";
    std::string opcodeString(1, hexDigits[opcode]);

    return instructionFactory.getInstruction(opcodeString);
}

// EXECUTE
void ControlUnit::execute(Instruction* instruction)
{
    if (instruction == nullptr)
    {
        throw std::invalid_argument("Invalid instruction");
    }

    instruction->execute(context);
}

// PROGRAM COUNTER
void ControlUnit::updatePC(int address)
{
    pc = address;
}

int ControlUnit::getPC() const
{
    return pc;
}

int ControlUnit::getIR() const
{
    return ir;
}

void ControlUnit::setIR(int value)
{
    ir = value;
}

void ControlUnit::setPC(int address)
{
    pc = address;
}

// DECODED OPERANDS
int* ControlUnit::getDecodedOperands()
{
    return decodedOperands;
}

