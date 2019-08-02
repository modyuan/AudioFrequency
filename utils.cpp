#include "utils.h"

#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#endif

void InitConsole(){
#ifdef _WIN32
    // Set console code page to UTF-8 so console known how to interpret string data
    SetConsoleOutputCP(CP_UTF8);

    // Enable buffering to prevent VS from chopping up UTF-8 byte sequences
    setvbuf(stdout, nullptr, _IOFBF, 1000);
#endif
}