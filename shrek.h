#ifndef _SHREK_H_INCLUDE_GUARD
#define _SHREK_H_INCLUDE_GUARD

#ifdef __cplusplus
extern "C"
#endif
{

#define SHREK_OK 0
#define SHREK_ERROR 1

// TODO: would be better to forward declare incomplete type?
typedef struct ShrekHandle
{
    void* runtime;
} ShrekHandle;

#ifdef __cplusplus
}
#endif

#endif // _SHREK_H_INCLUDE_GUARD
