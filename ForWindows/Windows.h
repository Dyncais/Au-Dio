//
// Created by Dima on 24.02.2024.
//

#pragma once
#include <string>
void SetConsoleOutputToUnicode();
std::wstring WstringFromString(const std::string& str);
std::string StringFromWstring(const std::wstring& str);