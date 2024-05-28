// utilities.h
#ifndef UTILITIES_H_INCLUDED
#define UTILITIES_H_INCLUDED

#include <string>
#include <vector>
using namespace std;
/*#ifndef is a preprocessor directive in C and C++ that stands
for "if not defined." It is typically used in conjunction with
 #define to create header guards, which prevent a header file
  from being included multiple times in the same translation unit
  (source file) during compilation.
 1. At the beginning of a header file, you place an #ifndef
  directive followed by a unique identifier, typically the name of
  the header file in all uppercase letters with underscores
  replacing spaces or special characters.

 2.Immediately after the #ifndef directive, you place an
  #define directive with the same unique identifier.

 3.At the end of the header file, you place an #endif directive.


  */
// Function declarations
void enableVirtualTerminalProcessing();
int getTerminalWidth();
void printCentered(const string& text);
void printColoredCenteredBlock(const string& colorCode, const vector<string>& lines);
void welcome_message();


#endif // UTILITIES_H_INCLUDED
