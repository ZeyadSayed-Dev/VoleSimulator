#ifndef IMETHOD_OPERATIONS_H
#define IMETHOD_OPERATIONS_H

#include <cstdint>

class IMethodOperations {
public:
    virtual uint8_t read(int address) = 0;
    virtual void write(int address, uint8_t value) = 0;
    virtual void clear() = 0;
    virtual ~IMethodOperations() {}
};

#endif 