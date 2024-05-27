#define _WIN32_WINNT 0x0A00 // Target Windows 10 or later

#include "utilities.h"
#include <iostream>
#include <windows.h>
#include <vector>

// Ensure ENABLE_VIRTUAL_TERMINAL_PROCESSING is defined
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
using namespace std;

void enableVirtualTerminalProcessing() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        cerr << "Error: Unable to get the console handle" << endl;
        return;
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) {
        cerr << "Error: Unable to get the console mode" << endl;
        return;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) {
        cerr << "Error: Unable to set the console mode" << endl;
        return;
    }
}
void printCentered(const string& text) {
    int width = 80; // Assume a fixed width for simplicity
    int padding = (width - text.size()) / 2;
    string paddingStr(padding, ' ');
    cout << paddingStr << text << endl;
}

void printColoredCenteredBlock(const string& colorCode, const vector<string>& lines) {
    cout << colorCode;
    for (const auto& line : lines) {
        printCentered(line);
    }
    cout << "\033[0m"; // Reset text color to default
}
//     cout << "\033[31m" << "This is red text" << "\033[0m" << endl;
//     cout << "\033[32m" << "This is green text" << "\033[0m" << endl;
//     cout << "\033[33m" << "This is yellow text" << "\033[0m" << endl;
//     cout << "\033[34m" << "This is blue text" << "\033[0m" << endl;
//     cout << "\033[35m" << "This is magenta text" << "\033[0m" << endl;
//     cout << "\033[36m" << "This is cyan text" << "\033[0m" << endl;
//     cout << "\033[1m\033[31m" << "This is bold red text" << "\033[0m" << endl;
//////////////////////////////////////////////////////////
//  cout << "\033[31m"; // Set text color to red

//     // Consecutive lines with the same color
//     cout << "This is red text on the first line" << endl;
//     cout << "This is red text on the second line" << endl;
//     cout << "This is red text on the third line" << endl;

//     cout << "\033[0m"; // Reset text color to default

//     cout << "This is default text color" << endl;