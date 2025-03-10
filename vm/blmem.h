#pragma once
#include "blstate.h"
#include "bltable.h"

class BLMem
{
public:
    static BLLuaState* NewLuaState();
    static BLTString* NewString(BLLuaState* L, LuaType::Type type);
    static BLTable* NewTable(BLLuaState* L);

    static void FreeObject(BLLuaState* L, GCObject* o, l_mem size);
};
