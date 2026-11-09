#ifndef VOLEMACHINE_H
#define VOLEMACHINE_H

#include <cstdint>
#include <string>
#include <vector>

#include "ALU.h"
#include "ControlUnit.h"
#include "MachineContext.h"
#include "Storage.h"
#include "Utilities.h"
#include "instructionFactory.hpp"

// How values written to the screen address (FF) are rendered as text.
enum class ScreenFormat
{
    Hex,      // Two uppercase hex digits: 0F, FC, 2A
    Decimal,  // Plain decimal number: 15, 252, 42
    Ascii     // Printable character, non-printable bytes as \xNN
};

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

    // Loads a program from a text file at an even start address.
    // The file is fully parsed and validated before the machine is reset, so a bad file
    // never leaves a half-loaded program behind. PC is set to startAddress.
    void loadProgramFile(const std::string &fileName, int startAddress = 0);

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

    // Raw bytes written to the screen address, in write order.
    const std::vector<uint8_t> &getScreenBytes() const;

    // Selects how getScreen() renders the bytes above.
    void setScreenFormat(ScreenFormat format);
    ScreenFormat getScreenFormat() const;

private:
    // Handles the "write to address FF is writing to the screen" mapping.
    void handleScreenWrite(uint16_t rawInstruction);

    // Renders one screen byte using the current ScreenFormat.
    std::string formatScreenByte(uint8_t value) const;

    // Rebuilds the cached text returned by getScreen().
    void rebuildScreenText() const;

    Memory memory;
    Register registerFile;
    ALU alu;
    OutputAndInput inputOutput;
    InstructionFactory instructionFactory;
    MachineContext context;
    ControlUnit controlUnit;

    bool running;
    ScreenFormat screenFormat;
    std::vector<uint8_t> screenBytes;
    mutable std::string screenText;
    mutable bool screenTextDirty;
};

#endif
