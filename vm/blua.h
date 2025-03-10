#pragma once
#include <cstddef>

#define LLONG_MAX
#if defined(LLONG_MAX)
using LUA_INTEGER = long long;
using LUA_NUMBER = double;
#else
using LUA_INTEGER = int;
using LUA_NUMBER = float;
#endif

typedef unsigned char lu_byte;
typedef size_t      lu_mem;
typedef ptrdiff_t   l_mem;