﻿#pragma once

#include "blgc.h"
#include "bltype.h"
#include "blua.h"

class BLLuaState;
// 使用类型别名提高可读性
using lua_Integer = LUA_INTEGER;
using lua_Number = LUA_NUMBER;
using lua_CFunction = int(*)(BLLuaState* L);

union BLValue
{
    GCObject* gc;
    void* p;
    int b;
    lua_Integer i;
    lua_Number n;
    lua_CFunction f;
};

class BLTValue
{
public:
    BLValue value;
    LuaType::Type type = LuaType::TNIL;
    bool EqualTo(BLTValue* other);
    bool IsNumber();
    bool IsNan();
    bool IsInteger();
    bool IsShrStr();
    bool IsNil();
};

extern inline BLTValue luaO_nilobject = { { nullptr }, LuaType::TNIL };
