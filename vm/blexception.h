#pragma once
#include <csetjmp>
#include "blobject.h"

enum class CALL_STATUS;
#define LUA_TRY(L, c, a) if (_setjmp((c)->b) == 0) { a } 

#ifdef _WINDOWS_PLATFORM_ 
#define LUA_THROW(c) longjmp((c)->b, 1) 
#else 
#define LUA_THROW(c) _longjmp((c)->b, 1) 
#endif

class lua_longjmp
{
public:
    lua_longjmp* previous;
    jmp_buf b;
    CALL_STATUS status;
};