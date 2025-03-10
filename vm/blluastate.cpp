#include "blluastate.h"

#include "blmem.h"
#include "blstate.h"
#include "bltype.h"
#include "blstring.h"

void BLLuaState::Init()
{
    topIndex = 0;
    AddNilToStack();
    
    ci = &base_ci;
    ci->func = 0;
    ci->top = MIN_STACK;
    ci->prev = ci->next = nullptr;
    nci = 0;

    marked = globalState->GetCurrentWhite();
    gclist = nullptr;
    type = LuaType::TTHREAD;

    prev = nullptr;
    status = CALL_OK;
    errorJmp = nullptr;
    errorfunc = 0;
}

lua_Integer BLLuaState::GetIntFromStack(int addr)
{
    lua_Integer ret = 0;
    BLTValue* value = GetTValueFromStackByAddr(addr);
    if(value->type != LuaType::TNUMBER)
    {
        // error
    }
    else
    {
        ret = value->value.i;
    }
    return ret;
}

char* BLLuaState::GetStringFromStack(int addr)
{
    BLTString* str = nullptr;
    BLTValue* value = GetTValueFromStackByAddr(addr);
    if(value->type != LuaType::TSTRING)
    {
        // error
    }
    else
    {
        str = (BLTString*)value->value.gc;
    }
    return str->data;
}

BLTValue* BLLuaState::GetTValueFromStackByAddr(int addr)
{
    int index = 0;
    if(addr >= 0)
    {
        index = ci->func + addr;
    }
    else
    {
        index = topIndex + addr;
    }

    if(index <= ci->func || index >= topIndex)
    {
        
    }
    
    BLTValue* value = GetTValueFromStackByIndex(index);
    return value;
}

BLTValue* BLLuaState::GetTValueFromStackByIndex(int index)
{
    if(index >= stack.size())
    {
        for(int i = stack.size(); i <= index; i++)
        {
            BLTValue* v = new BLTValue();
            stack.push_back(v);
        }
    }
    
    BLTValue* value = stack[index];
    return value;
}

void BLLuaState::AddIntToStack(lua_Integer value)
{
    BLTValue* v = AddToStack();
    v->value.i = value;
    v->type = LuaType::TNUMBER;
}

void BLLuaState::AddStringToStack(const char* str)
{
    BLTString* value = BLTString::NewString(this, str);
    BLTValue* v = AddToStack();
    v->value.gc = value;
    v->type = LuaType::TSTRING;
}

void BLLuaState::AddTableToStack(BLTable* table)
{
    BLTValue* v = AddToStack();
    v->value.gc = table;
    v->type = LuaType::TTABLE;
}

void BLLuaState::AddNilToStack()
{
    BLTValue* v = AddToStack();
    v->value.p = NULL;
    v->type = LuaType::TNIL;
}

void BLLuaState::AddCFuncToStack(lua_CFunction value)
{
    BLTValue* v = AddToStack();
    v->value.f = value;
    v->type = LuaType::TLCF;
}

void BLLuaState::Pop()
{
    topIndex--;
}

int BLLuaState::GetStackSize()
{
    return topIndex - (ci->func + 1);
}

int BLLuaState::SetTable(int idx)
{
    BLTValue* v = GetTValueFromStackByAddr(idx);
    BLTable* t = (BLTable*)v->value.gc;

    BLTValue* slot = NULL;
    BLTValue* key = GetTValueFromStackByIndex(topIndex - 2);
    BLTValue* value = GetTValueFromStackByIndex(topIndex - 1);
    BLTValue* oldValue = t->GetValue(this, key);
    if(oldValue == nullptr)
    {
    }
    else
    {
        
    }
    // bool isFastSet = false;
    // if (!luaV_fastset(this, t, topIndex - 2 , topIndex - 1, luaH_get, slot))
    //     luaV_finishset(this, t, topIndex - 2 , topIndex - 1, slot);}
    
    topIndex = topIndex - 2;
    return 1;
}


BLTValue* BLLuaState::AddToStack()
{
    BLTValue* v = nullptr;
    if(topIndex >= stack.size())
    {
        v = new BLTValue();
        stack.push_back(v);
    } else
    {
        v = stack[topIndex];
    }
    topIndex++;
    return v;
}

void BLLuaState::NewCI(int funcIndex, int nResults)
{
    CallInfo* nci = new CallInfo();
    ci->nResults = nResults;
    ci->func = funcIndex;
    ci->callStatus = CALL_STATUS::CALL_OK;
    ci->top = topIndex + MIN_STACK;
    ci->next = nci;
    nci->prev = ci;
    ci = nci;
}

int BLLuaState::PreCall(BLLuaState* L, int funcIndex, int nresult) {
    BLTValue* func = GetTValueFromStackByIndex(funcIndex);
    switch(func->type) {
    case LuaType::TLCF: {
        lua_CFunction f = func->value.f;
            
        //
        // ptrdiff_t func_diff = savestack(L, func);
        // luaD_checkstack(L, LUA_MINSTACK);
        // func = restorestack(L, func_diff);
        //
        NewCI(funcIndex, nresult);
        // next_ci(L, func, nresult);                        
        int n = (*f)(L);
        // assert(L->ci->func + n <= L->ci->top);
        PostCall(L, L->topIndex - n, n);
        return 1; 
    } break;
    default:break;
    }

    return 0;
}

int BLLuaState::PostCall(BLLuaState* L, int first_result, int nresult) {
    int func = ci->func;
    int nwant = ci->nResults;
    BLTValue* firstResult = GetTValueFromStackByIndex(first_result);
    switch(nwant) {
        case 0: {
            topIndex = func;
        } break;
        case 1: {
            if (nresult == 0) {
                firstResult->value.p = NULL;
                firstResult->type = LuaType::TNIL;
            }
            BLTValue* funcValue = stack[func];
            funcValue->value = firstResult->value;
            funcValue->type = firstResult->type;
            firstResult->value.p = NULL;
            firstResult->type = LuaType::TNIL;

            topIndex = func + nwant;
        } break;
        case -1: {
            int nres = L->topIndex - first_result;
            int i;
            for (i = 0; i < nres; i++) {
                BLTValue* newValue = stack[func + i];
                BLTValue* currentValue = stack[first_result + i];
                newValue->value = currentValue->value;
                newValue->type = currentValue->type;
                currentValue->value.p = NULL;
                currentValue->type = LuaType::TNIL;
            }
            topIndex = func + nres;
        } break;
        default: {
            if (nwant > nresult) {
                int i;
                for (i = 0; i < nwant; i++) {
                    BLTValue* newValue = stack[func + i];
                    if (i < nresult) {
                        BLTValue* currentValue = stack[first_result + i];
                        newValue->value = currentValue->value;
                        newValue->type = currentValue->type;
                        currentValue->value.p = NULL;
                        currentValue->type = LuaType::TNIL;
                    }
                    else {
                        newValue->value.p = NULL;
                        newValue->type = LuaType::TNIL;
                    }
                }
                topIndex = func + nwant;
            }
            else {
                int i;
                for (i = 0; i < nresult; i++) {
                    BLTValue* newValue = stack[func + i];
                    if (i < nwant) {
                        BLTValue* currentValue = stack[first_result + i];
                        newValue->value = currentValue->value;
                        newValue->type = currentValue->type;
                        currentValue->value.p = NULL;
                        currentValue->type = LuaType::TNIL;
                    }
                    else {
                        newValue->value.p = NULL;
                        newValue->type = LuaType::TNIL;
                    }
                }
                topIndex = func + nwant;
            } 
        } break;
    }

    CallInfo* ci = L->ci;
    L->ci = ci->prev;
    L->ci->next = NULL;
    
    BLGlobalState* g = L->globalState;
    delete ci;

    return CALL_STATUS::CALL_OK;
}


static int f_call(BLLuaState* L, void* ud) {
    CallS* c = (CallS*)ud;
    if(++L->nCalls > 20000) {
        // error
    }

    if(L->PreCall(L, c->funcIndex, c->nResults)) {
        // error
    }

    L->nCalls--;
    return CALL_STATUS::CALL_OK;
}
typedef int (*Pfunc)(BLLuaState* L, void* ud);
int BLLuaState::PCall(BLLuaState* ls, int nArgs, int nResults)
{
    int status = CALL_STATUS::CALL_OK;
    CallS c;
    c.funcIndex = ls->topIndex - nArgs - 1;
    c.nResults = nResults;

    // luaD_pcall
    CallInfo* old_ci = ls->ci;
    
    // luaD_rawrunprotected    
    int old_nCalls = ls->nCalls;
    lua_longjmp lj;
    lj.previous = ls->errorJmp;
    lj.status = CALL_STATUS::CALL_OK;
    ls->errorJmp = &lj;
    Pfunc f = &f_call;
    void* ud = &c;
    LUA_TRY(
        ls,
        ls->errorJmp,
        (*f)(ls, ud);
    )
    ls->errorJmp = lj.previous;
    ls->nCalls = old_nCalls;
    status = lj.status;
    
    if(status != CALL_STATUS::CALL_OK)
    {
        // error
    }

    return status;
}
