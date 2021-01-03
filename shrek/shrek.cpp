#include "shrek.h"

#include "shrek.h"
#include "shrek_runtime.h"

#ifdef __cplusplus
extern "C"
#endif
{

typedef struct ShrekHandle
{
    void* runtime;
} ShrekHandle;

shrek_API_FUNC(ShrekHandle*) shrek_new_runtime()
{
    auto shrek = new ShrekHandle;

    auto rt = new shrek::ShrekRuntime(shrek);
    shrek->runtime = (void*)rt;

    return shrek;
}

shrek_API_FUNC(void) shrek_free_runtime(ShrekHandle* shrek)
{
    if (shrek)
    {
        auto runtime = (shrek::ShrekRuntime*)shrek->runtime;
        delete runtime;
        delete shrek;
    }
}

shrek_API_FUNC(int) shrek_run(ShrekHandle* shrek, int argc, const char** argv)
{
    auto rt = (shrek::ShrekRuntime*)shrek->runtime;
    return rt->run(argc, argv);
}

shrek_API_FUNC(int) shrek_register_func(ShrekHandle* shrek, ShrekValue func_number, ShrekFunc func)
{
    if (!shrek)
    {
        return SHREK_ERROR;
    }

    auto rt = (shrek::ShrekRuntime*)shrek->runtime;
    if (!rt->register_function(func_number, func))
    {
        return SHREK_ERROR;
    }

    return SHREK_OK;
}

shrek_API_FUNC(void) shrek_set_except(ShrekHandle* shrek, const char* errmsg)
{
    if (!shrek)
    {
        return;
    }

    try
    {
        auto rt = (shrek::ShrekRuntime*)shrek->runtime;
        rt->set_func_exception(errmsg);
    }
    catch (...)
    {
        // TODO?
    }
}

shrek_API_FUNC(ShrekValue) shrek_stack_size(ShrekHandle* shrek)
{
    if (!shrek)
    {
        return SHREK_ERROR;
    }

    auto rt = (shrek::ShrekRuntime*)shrek->runtime;

    try
    {
        return (ShrekValue)rt->stack_size();
    }
    catch (...)
    {
        return 0;
    }
}

shrek_API_FUNC(int) shrek_pop(ShrekHandle* shrek, ShrekValue* out_value)
{
    if (shrek_peek(shrek, out_value) != SHREK_OK)
    {
        return SHREK_ERROR;
    }

    auto rt = (shrek::ShrekRuntime*)shrek->runtime;

    try
    {
       *out_value = rt->stack_pop();
       return SHREK_OK;
    }
    catch (...)
    {
        return SHREK_ERROR;
    }
}

shrek_API_FUNC(int) shrek_push(ShrekHandle* shrek, ShrekValue value)
{
    if (!shrek)
    {
        return SHREK_ERROR;
    }

    try
    {
        auto rt = (shrek::ShrekRuntime*)shrek->runtime;
        rt->stack_push(value);
        return SHREK_OK;
    }
    catch (...)
    {
        return SHREK_ERROR;
    }
}

shrek_API_FUNC(int) shrek_peek(ShrekHandle* shrek, ShrekValue* out_value)
{
    if (!shrek)
    {
        return SHREK_ERROR;
    }

    auto rt = (shrek::ShrekRuntime*)shrek->runtime;

    try
    {
        if (rt->stack_size() == 0)
        {
            out_value = 0;
            return SHREK_ERROR;
        }

        *out_value = rt->stack_peek();
        return SHREK_OK;
    }
    catch (...)
    {
        return SHREK_ERROR;
    }
}

#ifdef __cplusplus
}
#endif
