#ifndef MENU_H
#define MENU_H

#include <string>

#include "VoleMachine.h"

// Console user interface for the Vole machine simulator.
class Menu
{
public:
    Menu();
    void start();

private:
    VoleMachine machine;
    int entryAddress;

    void showMenu();
    void displayState();

    void loadNewProgramFlow();
    void enterInstructionsFlow();
    void runProgramFlow();
    void stepProgramFlow();

    static std::string readLine(const std::string &prompt);
    static int readInt(const std::string &prompt, int min, int max);
    static std::string trim(const std::string &text);
    static std::string hexByte(int value);
    static std::string hexWord(int value);
};

#endif
