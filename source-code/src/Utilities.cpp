#include "Utilities.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace {

    // Returns the value of a hex digit, or -1 when the character is not hex.
    int hexDigitValue(char character) {
        if (character >= '0' && character <= '9') {
            return character - '0';
        }
        if (character >= 'a' && character <= 'f') {
            return character - 'a' + 10;
        }
        if (character >= 'A' && character <= 'F') {
            return character - 'A' + 10;
        }
        return -1;
    }

    // Removes a leading UTF-8 byte order mark when a text editor added one.
    string withoutByteOrderMark(const string& text) {
        if (text.size() >= 3 &&
            static_cast<unsigned char>(text[0]) == 0xEF &&
            static_cast<unsigned char>(text[1]) == 0xBB &&
            static_cast<unsigned char>(text[2]) == 0xBF) {
            return text.substr(3);
        }
        return text;
    }

}

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
    validateProgram(programBytes, startAddress);

    for (size_t i = 0; i < programBytes.size(); ++i) {
        storage.write(startAddress + static_cast<int>(i), programBytes[i]);
    }
}

void ProgramLoader::clear(IMethodOperations& storage) {
    storage.clear();
}

vector<uint8_t> ProgramLoader::parseProgramText(const string& text) const {
    const string content = withoutByteOrderMark(text);

    string digits;
    string line;
    istringstream stream(content);

    while (getline(stream, line)) {
        // Anything after a semicolon is a comment.
        const size_t commentStart = line.find(';');
        if (commentStart != string::npos) {
            line.erase(commentStart);
        }

        for (size_t i = 0; i < line.size(); ++i) {
            const char character = line[i];

            if (isspace(static_cast<unsigned char>(character))) {
                continue;
            }

            if (hexDigitValue(character) < 0) {
                throw invalid_argument(
                    string("Invalid character '") + character +
                    "': only hex digits 0-9 and A-F are allowed.");
            }

            digits += character;
        }
    }

    if (digits.empty()) {
        throw invalid_argument("The program contains no instructions.");
    }

    if (digits.size() % 4 != 0) {
        ostringstream message;
        message << "Incomplete instruction: found " << digits.size()
                << " hex digits, which is not a multiple of 4.";
        throw invalid_argument(message.str());
    }

    // Every 4 hex characters form one 16-bit instruction, stored high byte
    // first in two consecutive memory cells.
    vector<uint8_t> programBytes;
    programBytes.reserve(digits.size() / 2);

    for (size_t i = 0; i < digits.size(); i += 4) {
        const int highByte =
            (hexDigitValue(digits[i]) << 4) | hexDigitValue(digits[i + 1]);
        const int lowByte =
            (hexDigitValue(digits[i + 2]) << 4) | hexDigitValue(digits[i + 3]);

        programBytes.push_back(static_cast<uint8_t>(highByte));
        programBytes.push_back(static_cast<uint8_t>(lowByte));
    }

    return programBytes;
}

vector<uint8_t> ProgramLoader::readProgramFile(const string& fileName) const {
    ifstream file(fileName.c_str());
    if (!file) {
        throw runtime_error("Cannot open program file: " + fileName);
    }

    ostringstream buffer;
    buffer << file.rdbuf();

    return parseProgramText(buffer.str());
}

void ProgramLoader::validateProgram(const vector<uint8_t>& programBytes, int startAddress) const {
    if (programBytes.empty()) {
        throw invalid_argument("The program is empty.");
    }

    if (startAddress < 0 || startAddress > 254) {
        throw out_of_range("Start address must be between 00 and FE.");
    }

    if (startAddress % 2 != 0) {
        throw invalid_argument(
            "Start address must be an even number (00, 02, 04, ... FE).");
    }

    if (static_cast<size_t>(startAddress) + programBytes.size() > 256) {
        ostringstream message;
        message << "Program does not fit in memory: it needs "
                << programBytes.size() << " bytes starting at "
                << startAddress << ", but memory ends at 255.";
        throw out_of_range(message.str());
    }
}

void ProgramLoader::loadProgramFile(IMethodOperations& storage, const string& fileName, int startAddress) {
    const vector<uint8_t> programBytes = readProgramFile(fileName);
    validateProgram(programBytes, startAddress);
    loadProgram(storage, programBytes, startAddress);
}
