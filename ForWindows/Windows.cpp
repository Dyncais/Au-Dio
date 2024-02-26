//
// Created by Dima on 24.02.2024.
//
#include "Windows.h"
#if defined(WIN32) || defined(_WIN32)
#include <windows.h>

#endif
void SetConsoleOutputToUnicode()
{
#if defined(WIN32) || defined(_WIN32)
    ::SetConsoleOutputCP(CP_UTF8);
#endif
}