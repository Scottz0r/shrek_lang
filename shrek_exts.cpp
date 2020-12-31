// TODO: This should really be in another project that builds a shared library.

#include "shrek_ext.h"

// TODO: I don't think this is the correct way to go about all of this.
#include "shrek_runtime.h"

#ifdef __cplusplus
extern "C"
#endif
{

int shrek_register_func(ShrekHandle* shrek, int func_number, ShrekFunc func)
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

void shrek_set_except(ShrekHandle* shrek, const char* errmsg)
{
    if (!shrek)
    {
        return;
    }

    auto rt = (shrek::ShrekRuntime*)shrek->runtime;

    // TODO: Implement this.
}

int shrek_stack_size(ShrekHandle* shrek)
{
    if (!shrek)
    {
        return SHREK_ERROR;
    }

    auto rt = (shrek::ShrekRuntime*)shrek->runtime;

    return (int)rt->stack().size();
}

int shrek_pop(ShrekHandle* shrek, int* out_value)
{
    if (shrek_peek(shrek, out_value) != SHREK_OK)
    {
        return SHREK_ERROR;
    }

    auto rt = (shrek::ShrekRuntime*)shrek->runtime;
    rt->stack().pop();

    return SHREK_OK;
}

int shrek_push(ShrekHandle* shrek, int value)
{
    if (!shrek)
    {
        return SHREK_ERROR;
    }

    auto rt = (shrek::ShrekRuntime*)shrek->runtime;
    rt->stack().push(value);
    return SHREK_OK;
}

int shrek_peek(ShrekHandle* shrek, int* out_value)
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
