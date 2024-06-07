//
// Created by Dima on 24.02.2024.
//
#include "Windows.h"
#include <stdexcept>
#if defined(WIN32) || defined(_WIN32)
#include <windows.h>

#endif
void SetConsoleOutputToUnicode()
{
#if defined(WIN32) || defined(_WIN32)
    ::SetConsoleOutputCP(CP_UTF8);
#endif
}

std::wstring WstringFromString(const std::string& str)
{
#if defined (_WIN32)
    if (str.empty())
        {
            return L"";
        }

        const auto size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
        if (size_needed <= 0)
        {
            // TODO: decide whether to throw or return empty string or report error some other way
            throw std::runtime_error("MultiByteToWideChar() failed: " + std::to_string(size_needed));
        }

        std::wstring result(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), result.data(), size_needed);
        return result;
#endif
    return {};
}
std::string StringFromWstring(const std::wstring& str)
{
#if defined (_WIN32)
    if (str.empty())
        {
            return "";
        }

        const auto size_needed =
            WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0, nullptr, nullptr);
        if (size_needed <= 0)
        {
            // TODO: decide whether to throw or return empty string or report error some other way
            throw std::runtime_error("WideCharToMultiByte() failed: " + std::to_string(size_needed));
        }

        std::string result(size_needed, 0);
        WideCharToMultiByte(
            CP_UTF8, 0, str.data(), (int)str.size(), result.data(), size_needed, nullptr, nullptr);
        return result;
#endif
    return {};
}