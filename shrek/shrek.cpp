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

shrek_API_FUNC(int) shrek_register_func(ShrekHandle* shrek, int func_number, ShrekFunc func)
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

    auto rt = (shrek::ShrekRuntime*)shrek->runtime;

    // TODO: Implement this.
}

shrek_API_FUNC(int) shrek_stack_size(ShrekHandle* shrek)
{
    if (!shrek)
    {
        return SHREK_ERROR;
    }

    auto rt = (shrek::ShrekRuntime*)shrek->runtime;

    return (int)rt->stack().size();
}

shrek_API_FUNC(int) shrek_pop(ShrekHandle* shrek, int* out_value)
{
    if (shrek_peek(shrek, out_value) != SHREK_OK)
    {
        return SHREK_ERROR;
    }

    auto rt = (shrek::ShrekRuntime*)shrek->runtime;
    rt->stack().pop();

    return SHREK_OK;
}

shrek_API_FUNC(int) shrek_push(ShrekHandle* shrek, int value)
{
    if (!shrek)
    {
        return SHREK_ERROR;
    }

    auto rt = (shrek::ShrekRuntime*)shrek->runtime;
    rt->stack().push(value);
    return SHREK_OK;
}

shrek_API_FUNC(int) shrek_peek(ShrekHandle* shrek, int* out_value)
{
    if (!shrek)
    {
        return SHREK_ERROR;
    }

    auto rt = (shrek::ShrekRuntime*)shrek->runtime;

    if (rt->stack().empty())
    {
        out_value = 0;
        return SHREK_ERROR;
    }

    *out_value = rt->stack().top();

    return SHREK_OK;
}

#ifdef __cplusplus
}
#endif
