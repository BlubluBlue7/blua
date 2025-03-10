#pragma once
#include <vector>

#include "blexception.h"
#include "blobject.h"
#include "blgc.h"
#include "bltable.h"

class BLTValue;
class BLGlobalState;

class CallInfo
{
public:
    int func;
    int top;
    int nResults;
    int callStatus;
    CallInfo* next;
    CallInfo* prev;
};

class BLLuaState: public GCObject
{
public:
    std::vector<BLTValue*> stack;
    int topIndex;

    lua_longjmp* errorJmp;

    int status;
    BLLuaState* prev;
    
    CallInfo* ci;
    
    CallInfo base_ci;
    int nci;
    int nCalls;
    GCObject* gclist;
    
    BLGlobalState* globalState;
    
    ptrdiff_t errorfunc;
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

    int PCall(BLLuaState* ls, int nArgs, int nResults);
    void NewCI(int funcIndex, int nResults);

    int PreCall(BLLuaState* L, int funcIndex, int nresult);
    int PostCall(BLLuaState* L, int first_result, int nresult);
};
