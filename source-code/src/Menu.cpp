#include "Menu.h"

#include <cctype>
#include <iostream>
#include <stdexcept>

namespace
{
    char hexDigitChar(int value)
    {
        static const char digits[] = "0123456789ABCDEF";
        return digits[value & 0x0F];
    }
}

Menu::Menu()
    : machine(),
      entryAddress(0),
      programStartAddress(0),
      programLoaded(false)
{
}

void Menu::start()
{
    std::cout << "=======================================================\n"
              << "               VOLE MACHINE SIMULATOR\n"
              << "=======================================================\n";

    bool quit = false;

    while (!quit)
    {
        showMenu();

        const int choice = readInt("Choose an option: ", 1, 5);

        try
        {
            switch (choice)
            {
                case 1:
                    loadNewProgramFlow();
                    break;

                case 2:
                    loadProgramFileFlow();
                    break;

                case 3:
                    runProgramFlow();
                    break;

                case 4:
                    stepProgramFlow();
                    break;

                case 5:
                    quit = true;
                    break;

                default:
                    break;
            }
        }
        catch (const std::exception &error)
        {
            std::cout << "\n[Error] " << error.what() << "\n";
        }
    }

    std::cout << "\nGoodbye.\n";
}

void Menu::showMenu()
{
    std::cout << "\n----------------------- MENU ------------------------\n"
              << "  1. Load new program (type instructions)\n"
              << "  2. Load program from a file\n"
              << "  3. Run program as a whole\n"
              << "  4. Run step by step\n"
              << "  5. Exit program\n"
              << "-----------------------------------------------------\n";
}

void Menu::displayState()
{
    Memory &memory = machine.getMemory();
    Register &registers = machine.getRegisterFile();
    ControlUnit &controlUnit = machine.getControlUnit();

    std::cout << "\n==================== MACHINE STATE ====================\n";

    std::cout << "Memory (rows = high nibble, columns = low nibble):\n";
    std::cout << "      ";
    for (int column = 0; column < 16; ++column)
    {
        std::cout << hexByte(column) << " ";
    }
    std::cout << "\n";

    for (int row = 0; row < 16; ++row)
    {
        std::cout << "  " << hexByte(row) << ": ";
        for (int column = 0; column < 16; ++column)
        {
            const int address = row * 16 + column;
            std::cout << hexByte(memory.read(address)) << " ";
        }
        std::cout << "\n";
    }

    std::cout << "\nRegisters:\n";
    for (int index = 0; index < 16; ++index)
    {
        std::cout << "  R" << hexDigitChar(index) << "="
                  << hexByte(registers.read(index));
        if (index % 8 == 7)
        {
            std::cout << "\n";
        }
    }

    std::cout << "\nPC = " << hexWord(controlUnit.getPC())
              << "    IR = " << hexWord(controlUnit.getIR()) << "\n";

    std::cout << "\nScreen output:\n";
    if (machine.getScreen().empty())
    {
        std::cout << "  (empty)\n";
    }
    else
    {
        std::cout << machine.getScreen() << "\n";
    }

    std::cout << "=======================================================\n";
}

void Menu::loadNewProgramFlow()
{
    std::cout << "\nLoading a new program...\n";

    // A new program replaces whatever was in the machine before.
    machine.reset();
    entryAddress = 0;
    programStartAddress = 0;
    programLoaded = false;

    enterInstructionsFlow();

    if (entryAddress == 0)
    {
        std::cout << "No instructions entered.\n";
        return;
    }

    programLoaded = true;

    std::cout << "Program loaded: " << (entryAddress / 2)
              << " instruction(s) from 0000 to " << hexWord(entryAddress - 2)
              << ".\nChoose 3 to run it or 4 to run it step by step.\n";
}

void Menu::loadProgramFileFlow()
{
    std::cout << "\nLoading a program from a file...\n";

    const std::string fileName = trim(readLine("Enter the program file path: "));
    if (fileName.empty())
    {
        std::cout << "No file name entered.\n";
        return;
    }

    const std::string addressText =
        trim(readLine("Enter the start address in hex (even, 00-FE, default 00): "));

    int startAddress = 0;
    if (!addressText.empty())
    {
        startAddress = parseHexValue(addressText);
    }

    // VoleMachine reads and validates the whole file before resetting, so a
    // bad file leaves the current program untouched.
    machine.loadProgramFile(fileName, startAddress);

    programStartAddress = startAddress;
    entryAddress = startAddress;
    programLoaded = true;

    std::cout << "Program loaded from \"" << fileName << "\" at address "
              << hexWord(startAddress) << ".\n"
              << "PC = " << hexWord(machine.getControlUnit().getPC())
              << ".\nChoose 3 to run it or 4 to run it step by step.\n";

    displayState();
}

void Menu::enterInstructionsFlow()
{
    std::cout << "\nEnter one instruction per line as 4 hex digits (e.g. 2207).\n"
              << "Press ENTER on an empty line to go back to the menu.\n";

    while (true)
    {
        std::cout << "[" << hexWord(entryAddress) << "] > ";

        std::string line;
        if (!std::getline(std::cin, line))
        {
            return;
        }

        line = trim(line);

        if (line.empty())
        {
            return;
        }

        if (line.size() != 4)
        {
            std::cout << "Invalid instruction: enter exactly 4 hex digits (e.g. 2207).\n";
            continue;
        }

        bool valid = true;
        for (std::size_t i = 0; i < line.size(); ++i)
        {
            if (!std::isxdigit(static_cast<unsigned char>(line[i])))
            {
                valid = false;
                break;
            }
        }

        if (!valid)
        {
            std::cout << "Invalid instruction: only hex digits 0-9 and A-F are allowed.\n";
            continue;
        }

        if (entryAddress > 254)
        {
            std::cout << "Memory is full.\n";
            return;
        }

        const int value = std::stoi(line, nullptr, 16);

        machine.getMemory().write(entryAddress,
                                  static_cast<uint8_t>((value >> 8) & 0xFF));
        machine.getMemory().write(entryAddress + 1,
                                  static_cast<uint8_t>(value & 0xFF));

        entryAddress += 2;

        // Show the machine state after every instruction that is written.
        displayState();
    }
}

void Menu::runProgramFlow()
{
    if (!programLoaded)
    {
        std::cout << "No program has been loaded yet. Choose option 1 or 2 first.\n";
        return;
    }

    // Start from the first instruction of the loaded program.
    machine.getControlUnit().setPC(programStartAddress);
    machine.start();

    std::cout << "\nRunning program as a whole...\n";
    machine.run();
    std::cout << "\nProgram finished.\n";

    displayState();
}

void Menu::stepProgramFlow()
{
    if (!programLoaded)
    {
        std::cout << "No program has been loaded yet. Choose option 1 or 2 first.\n";
        return;
    }

    // Start from the first instruction of the loaded program.
    machine.getControlUnit().setPC(programStartAddress);
    machine.start();

    while (machine.isRunning())
    {
        machine.step();
        displayState();

        if (!machine.isRunning())
        {
            break;
        }

        std::cout << "\nPress ENTER to execute the next instruction...";
        std::string line;
        std::getline(std::cin, line);
    }

    std::cout << "\nProgram finished.\n";
}

std::string Menu::readLine(const std::string &prompt)
{
    std::cout << prompt;

    std::string line;
    if (!std::getline(std::cin, line))
    {
        throw std::runtime_error("End of input.");
    }

    return line;
}

int Menu::readInt(const std::string &prompt, int min, int max)
{
    while (true)
    {
        const std::string input = trim(readLine(prompt));

        try
        {
            std::size_t used = 0;
            const int value = std::stoi(input, &used);

            if (used == input.size() && value >= min && value <= max)
            {
                return value;
            }
        }
        catch (const std::exception &)
        {
            // Fall through to the error message below.
        }

        std::cout << "Please enter a number between " << min << " and " << max << ".\n";
    }
}

std::string Menu::trim(const std::string &text)
{
    const std::size_t first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
    {
        return "";
    }

    const std::size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

int Menu::parseHexValue(const std::string &text)
{
    if (text.empty())
    {
        throw std::invalid_argument("Empty hex value.");
    }

    std::size_t used = 0;
    long value = 0;

    try
    {
        value = std::stol(text, &used, 16);
    }
    catch (const std::exception &)
    {
        throw std::invalid_argument("Invalid hex value: " + text);
    }

    if (used != text.size() || value < 0 || value > 0xFFFF)
    {
        throw std::invalid_argument("Invalid hex value: " + text);
    }

    return static_cast<int>(value);
}

std::string Menu::hexByte(int value)
{
    std::string result;
    result += hexDigitChar((value >> 4) & 0x0F);
    result += hexDigitChar(value & 0x0F);
    return result;
}

std::string Menu::hexWord(int value)
{
    if (value < 0)
    {
        return "----";
    }

    std::string result;
    result += hexDigitChar((value >> 12) & 0x0F);
    result += hexDigitChar((value >> 8) & 0x0F);
    result += hexDigitChar((value >> 4) & 0x0F);
    result += hexDigitChar(value & 0x0F);
    return result;
}
