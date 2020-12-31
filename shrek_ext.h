#ifndef _SHREK_EXT_H_INCLUDE_GUARD
#define _SHREK_EXT_H_INCLUDE_GUARD

#include "shrek.h"

#ifdef __cplusplus
extern "C"
#endif
{

typedef int (*ShrekFunc)(ShrekHandle*);

int shrek_register_func(ShrekHandle* shrek, int func_number, ShrekFunc func);

void shrek_set_except(ShrekHandle* shrek, const char* errmsg);

int shrek_stack_size(ShrekHandle* shrek);

int shrek_pop(ShrekHandle* shrek, int *out_value);

int shrek_push(ShrekHandle* shrek, int value);

int shrek_peek(ShrekHandle* shrek, int *out_value);

#ifdef __cplusplus
}
#endif

#endif // _SHREK_EXT_H_INCLUDE_GUARD
