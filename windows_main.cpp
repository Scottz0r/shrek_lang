#ifndef _WIN32
#error Incorrect platform
#endif

#include <fstream>
#include <sstream>
#include <Windows.h>

#include "shrek_interpreter.h"
#include "shrek_runtime.h"

int wmain(int argc, const wchar_t** argv)
{
    // To make output work with UTF-8. Console must have supporting character set.
    SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IOFBF, 4096);

    // TODO: Argument parsing (max stack size options and such?)
    if (argc < 2)
    {
        return 1;
    }

    try
    {
        std::ifstream fp(argv[1], std::ios::in | std::ios::binary);
        std::stringstream ss;
        ss << fp.rdbuf();
        fp.close();

        auto code = shrek::interpret_code(ss.str());
        shrek::execute(code);


        // shrek::execute();
    }
    catch (...)
    {
        // TODO - Move this to interperter and runtime.
    }

    return 0;
}
