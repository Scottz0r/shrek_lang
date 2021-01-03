#ifndef _WIN32
#error Incorrect platform
#endif

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <Windows.h>

#include "shrek.h"
#include "shrek_builtins.h"

bool utf8_to_utf16(const std::string& utf8, std::wstring& out_utf16);
bool utf16_to_utf8(const std::wstring& utf16, std::string& out_utf8);

int wmain(int argc, const wchar_t** argv)
{
    // To make output work with UTF-8. Console must have supporting character set.
    SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IOFBF, 4096);

    auto utf8_args_strs = std::make_unique<std::string[]>(argc);
    auto utf8_argv = std::make_unique<const char* []>(argc);

    for (int i = 0; i < argc; ++i)
    {
        if (!utf16_to_utf8(argv[i], utf8_args_strs[i]))
        {
            std::cout << "argument unicode error" << std::endl;
            return -127;
        }

        utf8_argv[i] = utf8_args_strs[i].c_str();
    }

    auto shrek = shrek_new_runtime();
    if (!shrek)
    {
        std::cout << "Shrek runtime initialization failure" << std::endl;
        return -1;
    }

    int rc = shrek_run(shrek, argc, utf8_argv.get());
    shrek_free_runtime(shrek);
    return rc;
}

bool utf8_to_utf16(const std::string& utf8, std::wstring& out_utf16)
{
    constexpr auto max_size = (1 << 16) - 1;

    if (utf8.size() > max_size)
    {
        return false;
    }

    int needed_mb_chars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.c_str(), (int)utf8.size(), nullptr, 0);

    auto buffer = new wchar_t[needed_mb_chars];

    int rc = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.c_str(), (int)utf8.size(), buffer, needed_mb_chars);

    auto result = false;
    if (rc > 0)
    {
        out_utf16.assign(buffer, needed_mb_chars);
        result = true;
    }

    delete[] buffer;
    return result;
}

bool utf16_to_utf8(const std::wstring& utf16, std::string& out_utf8)
{
    constexpr auto max_size = (1 << 16) - 1;

    if (utf16.size() > max_size)
    {
        return false;
    }

    int needed_chars = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, utf16.c_str(), (int)utf16.size(), nullptr, 0, nullptr, nullptr);

    auto buffer = new char[needed_chars];

    int rc = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, utf16.c_str(), (int)utf16.size(), buffer, needed_chars, nullptr, nullptr);

    auto result = false;
    if (rc > 0)
    {
        out_utf8.assign(buffer, needed_chars);
        result = true;
    }

    delete[] buffer;
    return result;
}
