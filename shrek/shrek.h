#ifndef _SHREK_H_INCLUDE_GUARD
#define _SHREK_H_INCLUDE_GUARD

#include "shrek_exports.h"

#ifdef __cplusplus
extern "C"
#endif
{

#define SHREK_OK 0
#define SHREK_ERROR 1

typedef struct ShrekHandle ShrekHandle;

typedef int (*ShrekFunc)(ShrekHandle*);

// Runtime API
shrek_API_FUNC(ShrekHandle*) shrek_new_runtime();

shrek_API_FUNC(void) shrek_free_runtime(ShrekHandle* shrek);

shrek_API_FUNC(int) shrek_run(ShrekHandle* shrek, int argc, const char **argv);

shrek_API_FUNC(int) shrek_register_func(ShrekHandle* shrek, int func_number, ShrekFunc func);

shrek_API_FUNC(void) shrek_set_except(ShrekHandle* shrek, const char* errmsg);

shrek_API_FUNC(int) shrek_stack_size(ShrekHandle* shrek);

shrek_API_FUNC(int) shrek_pop(ShrekHandle* shrek, int* out_value);

shrek_API_FUNC(int) shrek_push(ShrekHandle* shrek, int value);

shrek_API_FUNC(int) shrek_peek(ShrekHandle* shrek, int* out_value);

// Parser API - TODO

#ifdef __cplusplus
}
#endif

#endif // _SHREK_H_INCLUDE_GUARD
