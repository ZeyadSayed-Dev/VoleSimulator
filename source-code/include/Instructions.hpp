#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include "MachineContext.h"

class Instruction
{
public:
    virtual ~Instruction() = default;
    virtual void execute(MachineContext &context) = 0;
};

class LoadFromMemory : public Instruction
{
public:
    void execute(MachineContext &context) override
    {
        int *ptr = context.getControlUnit().getDecodedOperands();
        int8_t value = context.getMemory().read(ptr[1]);
        context.getRegisterFile().write(ptr[0], value);
    }
};

class LoadImmediate : public Instruction
{
public:
    void execute(MachineContext &context) override
    {
        int *ptr = context.getControlUnit().getDecodedOperands();
        int memberRegister = ptr[0];
        int8_t currentvalue = (int8_t)ptr[1];
        context.getRegisterFile().write(memberRegister, currentvalue);
    }
};

class Store : public Instruction
{
public:
    void execute(MachineContext &context) override
    {
        int *ptr = context.getControlUnit().getDecodedOperands();
        int8_t value = context.getRegisterFile().read(ptr[0]);
        context.getMemory().write(ptr[1], value);
    }
};

class Move : public Instruction
{
public:
    void execute(MachineContext &context) override
    {
        int *ptr = context.getControlUnit().getDecodedOperands();
        int8_t value = context.getRegisterFile().read(ptr[0]);
        context.getRegisterFile().write(ptr[1], value);
    }
};

class AddInteger : public Instruction
{
public:
    void execute(MachineContext &context) override
    {
        int *ptr = context.getControlUnit().getDecodedOperands();
        int8_t value1 = context.getRegisterFile().read(ptr[1]);
        int8_t value2 = context.getRegisterFile().read(ptr[2]);
        int8_t result = context.getALU().addInteger(value1, value2);
        context.getRegisterFile().write(ptr[0], result);
    }
};

class AddFloatingPoint : public Instruction
{
public:
    void execute(MachineContext &context) override
    {
        int *ptr = context.getControlUnit().getDecodedOperands();
        int8_t value1 = context.getRegisterFile().read(ptr[1]);
        int8_t value2 = context.getRegisterFile().read(ptr[2]);
        int8_t result = context.getALU().addFloatingPoint(value1, value2);
        context.getRegisterFile().write(ptr[0], result);
    }
};

class BitwiseOR : public Instruction
{
public:
    void execute(MachineContext &context) override
    {
        int *ptr = context.getControlUnit().getDecodedOperands();
        int8_t value1 = context.getRegisterFile().read(ptr[1]);
        int8_t value2 = context.getRegisterFile().read(ptr[2]);
        int8_t result = context.getALU().bitwiseOR(value1, value2);
        context.getRegisterFile().write(ptr[0], result);
    }
};

class BitwiseAND : public Instruction
{
public:
    void execute(MachineContext &context) override
    {
        int *ptr = context.getControlUnit().getDecodedOperands();
        int8_t value1 = context.getRegisterFile().read(ptr[1]);
        int8_t value2 = context.getRegisterFile().read(ptr[2]);
        int8_t result = context.getALU().bitwiseAND(value1, value2);
        context.getRegisterFile().write(ptr[0], result);
    }
};

class BitwiseXOR : public Instruction
{
public:
    void execute(MachineContext &context) override
    {
        int *ptr = context.getControlUnit().getDecodedOperands();
        int8_t value1 = context.getRegisterFile().read(ptr[1]);
        int8_t value2 = context.getRegisterFile().read(ptr[2]);
        int8_t result = context.getALU().bitwiseXOR(value1, value2);
        context.getRegisterFile().write(ptr[0], result);
    }
};

class RotateRight : public Instruction
{
public:
    void execute(MachineContext &context) override
    {
        int *ptr = context.getControlUnit().getDecodedOperands();
        int8_t value = context.getRegisterFile().read(ptr[0]);
        int8_t result = context.getALU().rotateRight(value, ptr[1]);
        context.getRegisterFile().write(ptr[0], result);
    }
};

class Jump : public Instruction
{
public:
    void execute(MachineContext &context) override
    {
        int *ptr = context.getControlUnit().getDecodedOperands();
        int8_t value1 = context.getRegisterFile().read(ptr[0]);
        int8_t value2 = context.getRegisterFile().read(0);
        if (value1 == value2)
        {
            context.getControlUnit().updatePC(ptr[1]);
        }
    }
};

class Halt : public Instruction
{
public:
    void execute(MachineContext &context) override
    {
        context.getControlUnit().updatePC(-1);
    }
};

#endif
