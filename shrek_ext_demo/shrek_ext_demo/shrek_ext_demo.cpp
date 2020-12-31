#include <shrek.h>

#include <stdio.h>

extern "C"
{
    int demo_func(ShrekHandle* shrek)
    {
        puts("This is from an extension!\n");
        return SHREK_OK;
    }

    __declspec(dllexport) int shrek_ext_demo_register(ShrekHandle* shrek)
    {
        int rc = shrek_register_func(shrek, 100, demo_func);
        return rc;
    }
}
