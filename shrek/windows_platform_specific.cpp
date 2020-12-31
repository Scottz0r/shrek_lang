#ifndef _WIN32
#error Incorrect platform
#endif

#include <fstream>
#include <sstream>
#include <Windows.h>
#include <filesystem>
#include "fmt/color.h"

#include "shrek.h"
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
    namespace fs = std::filesystem;

    void load_module(ShrekHandle* shrek, const fs::path& file);

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

    void discover_modules(ShrekHandle* shrek)
    {
        for (const auto& file : fs::directory_iterator(fs::current_path()))
        {
            auto ext = file.path().extension();
            if (ext == L".dnky")
            {
                load_module(shrek, file.path());
            }
        }
    }

    void load_module(ShrekHandle* shrek, const fs::path& file)
    {
        HMODULE handle = LoadLibraryW(file.c_str());
        if (handle == NULL)
        {
            std::string utf8_fn;
            utf16_to_utf8(file, utf8_fn);
            fmt::print("Failed to load module \"{}\"\n", utf8_fn);
            return;
        }

        // Make the procedure name based off of the file name, with the extension removed.
        auto proc_name = file.filename().u8string();
        proc_name = proc_name.substr(0, proc_name.size() - 5) + "_register";

        FARPROC register_func_raw = GetProcAddress(handle, proc_name.c_str());
        if (register_func_raw == NULL)
        {
            std::string utf8_fn;
            utf16_to_utf8(file, utf8_fn);
            fmt::print("Failed to register functions in \"{}\"\n", utf8_fn);

            FreeLibrary(handle);
            return;
        }

        ShrekRegister register_func = (ShrekRegister)register_func_raw;
        int rc = register_func(shrek);

        if (rc != SHREK_OK)
        {
            std::string utf8_fn;
            utf16_to_utf8(file, utf8_fn);
            fmt::print("Failed register function in \"{}\" returned unsuccessful return code\n", utf8_fn);

            FreeLibrary(handle);
            return;
        }

        // TODO: Storing modules for unloading?
    }
}
