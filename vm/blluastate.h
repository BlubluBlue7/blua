#pragma once
#include <vector>

#include "blexception.h"
#include "blobject.h"
#include "blgc.h"
#include "bltable.h"
#include "blstate.h"

class BLTValue;
class BLGlobalState;

class CallInfo
{
public:
    int func;
    int top;
    int nResults;
    CALL_STATUS callStatus;
    CallInfo* next;
    CallInfo* prev;
};

class BLLuaState: public GCObject
{
public:
    std::vector<BLTValue*> stack;
    int topIndex;

    lua_longjmp* errorJmp;
    ptrdiff_t errorfunc;
    BLGlobalState* globalState;

    CALL_STATUS status;
    BLLuaState* prev;
    CallInfo* ci;
    CallInfo base_ci;
    int nci;
    int nCalls;
    GCObject* gclist;
    
    void Init();
    int SetTable(int idx);
    lua_Integer GetIntFromStack(int addr);
    char* GetStringFromStack(int addr);
    BLTValue* GetTValueFromStackByAddr(int addr);
    BLTValue* GetTValueFromStackByIndex(int index);
    void AddIntToStack(lua_Integer value);
    void AddStringToStack(const char* str);
    void AddTableToStack(BLTable* table);
    void AddNilToStack();
    void AddCFuncToStack(lua_CFunction value);
    BLTValue* AddToStack();
    void Pop();
    int GetStackSize();
    void NewCI(int funcIndex, int nResults);
    CALL_STATUS PCall(BLLuaState* ls, int nArgs, int nResults);
    int PreCall(BLLuaState* L, int funcIndex, int nresult);
    CALL_STATUS PostCall(BLLuaState* L, int first_result, int nresult);
};
