#ifndef _WIN32
#error Incorrect platform
#endif

#include <fstream>
#include <sstream>
#include <Windows.h>

#include "shrek_platform_specific.h"

static bool utf8_to_utf16(const std::string& utf8, std::wstring& out_utf16);
static bool utf16_to_utf8(const std::wstring& utf16, std::string& out_utf8);

static bool utf8_to_utf16(const std::string& utf8, std::wstring& out_utf16)
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

static bool utf16_to_utf8(const std::wstring& utf16, std::string& out_utf8)
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

namespace shrek
{
    bool read_all_text(const std::string& utf8_filename, std::string& result)
    {
        std::wstring win_filename;
        if(!utf8_to_utf16(utf8_filename, win_filename))
        {
            return false;
        }

        std::ifstream fp(win_filename, std::ios::in | std::ios::binary);
        if (fp.is_open())
        {
            std::stringstream ss;
            ss << fp.rdbuf();
            result = ss.str();

            return true;
        }

        return false;
    }
}
