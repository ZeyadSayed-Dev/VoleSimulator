#include "Utilities.h"

using namespace std;

void OutputAndInput::write(const string& out) {
    output = out;
    cout << output << endl;
}

string OutputAndInput::read() {
    cout << "Enter input: ";
    cin >> input;
    return input;
}

void OutputAndInput::clear() {
    input.clear();
    output.clear();
}

void ProgramLoader::loadProgram(IMethodOperations& storage, const std::vector<uint8_t>& programBytes, int startAddress) {
    for (size_t i = 0; i < programBytes.size(); ++i) {
        storage.write(startAddress + i, programBytes[i]);
    }
}

void ProgramLoader::clear(IMethodOperations& storage) {
    storage.clear();
}