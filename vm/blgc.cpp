#include "blgc.h"

#include "blmem.h"
#include "blstate.h"
#include "bltype.h"

GCObject* GCObject::NewObj(BLLuaState* L, int type, size_t size)
{
    BLGlobalState* g = L->globalState;
    GCObject* obj = new GCObject();
    obj->marked = 0;
    obj->next = g->allgc;
    obj->type = type;
    g->allgc = obj;
    
    return obj;
}

void GCObject::ChangeWhite()
{
    marked ^= WHITE_BITS;
}

void BLGC::CheckGC(BLLuaState* L)
{
    if (L->globalState->GCdebt > 0) {
        StepGC(L);
    }
}

void BLGC::StepGC(BLLuaState* L)
{
    BLGlobalState* g = L->globalState;
    l_mem debt = g->GetDebt();
    do
    {
        l_mem work = SingleStep(L); // 本次完成的工作量
        debt -= work;
    } while(debt > -BLGlobalState::GCSTEPSIZE && g->gcstate != State::Pause); // 未完成债务，且 GC 未暂停

    if (g->gcstate == State::Pause) {
        g->SetPause(); // 设置暂停参数（如调整下次触发阈值）
    }
    else {
        // GC 未完成，重新计算债务并更新
        debt = g->GCdebt / g->GCstepmul * BLGlobalState::STEPMULADJ;
        g->SetDebt(debt);
    }
}

void BLGC::MarkValue(BLLuaState* lua_state, BLTValue* value)
{
    if(!is_collectable(value))
    {
        return;
    }

    GCObject* obj = value->value.gc;
    MarkObject(lua_state, obj);
}

void BLGC::MarkObject(BLLuaState* lua_state, GCObject* obj) {

    BLGlobalState* g = lua_state->globalState;
    if (GCHelper::is_white(obj)) {
        GCHelper::reset_bits(obj->marked, WHITE_BITS);
        switch (obj->type)
        {
        case LuaType::TTHREAD:
            {
                BLLuaState* L = (BLLuaState*)(obj);
                L->gclist = g->gray;
                g->gray = L;
                break;
            }
        case LuaType::TSTRING:
            {
                GCHelper::set_bits(obj->marked, BLACK_BIT);
                g->GCmemtrav += sizeof(BLTString);
                break;
            }
        }
    }
}

l_mem BLGC::SingleStep(BLLuaState* L)
{
    BLGlobalState* g = L->globalState;
    g->GCmemtrav = 0;
    switch (g->gcstate)
    {
    case State::Pause:
        {
            g->gray = nullptr;
            g->grayagain = nullptr;
            MarkObject(L, g->mainThread);
            g->gcstate = State::Propagate;
            break;    
        }
    case State::Propagate:
        {
            PropagateMark(L);
            if(g->gray == nullptr)
            {
                g->gcstate = State::Atomic;
            }
            break;
        }
    case State::Atomic:
        {
            if(g->gray)
            {
                PropagateAll(L);
            }
            Atomic(L);
            g->gcstate = State::SweepAllGC;
            g->sweepgc = SweepList(L, &g->allgc, 1);
            g->GCestimate = g->totalbytes + g->GCdebt;
            break;
        }
    case State::SweepAllGC:
        {
            SweepStep(L);
            break;
        }
    case State::SweepEnd:
        {
            g->gcstate = State::Pause;
            break;
        }
    }

    return g->GCmemtrav;
}

void BLGC::PropagateAll(BLLuaState* L)
{
    BLGlobalState* g = L->globalState;
    while(g->gray)
    {
        PropagateMark(L);
    }
}

void BLGC::PropagateMark(BLLuaState* L)
{
    BLGlobalState* g = L->globalState;
    if(!g->gray)
    {
        return;
    }
    GCObject* gco = g->gray;
    GCHelper::set_bits(gco->marked, BLACK_BIT);
    lu_mem size = 0;
    switch (gco->type)
    {
    case LuaType::TTHREAD:
        {
            GCHelper::reset_bits(gco->marked, BLACK_BIT);
            BLLuaState* L = (BLLuaState*)(gco);
            g->gray = L->gclist;
            L->gclist = g->grayagain;
            g->grayagain = L;
            int index = 0;
            for(; index < L->topIndex; ++index)
            {
                MarkValue(L, L->GetTValueFromStackByIndex(index));
            }
            size = sizeof(BLLuaState) + sizeof(BLTValue) * L->topIndex + sizeof(CallInfo) * L->nci;
            break;
        }
    }

    g->GCmemtrav += size;
}

void BLGC::Atomic(BLLuaState* L)
{
    BLGlobalState* g = L->globalState;
    g->gray = g->grayagain;
    g->grayagain = nullptr;
    g->gcstate = State::InsideAtomic;
    PropagateAll(L);
    g->currentwhite = g->GetOtherWhite();
}

GCObject** BLGC::SweepList(BLLuaState* L, GCObject** p, size_t count)
{
    BLGlobalState* g = L->globalState;
    lu_byte ow = g->GetOtherWhite();
    while(*p != nullptr && count > 0)
    {
        lu_byte marked = (*p)->marked;
        if(GCHelper::is_dead(ow, marked))
        {
            GCObject* gco = *p;
            *p = (*p)->next;
            g->GCmemtrav += FreeObj(L, gco);
        } else
        {
            (*p)->marked &=  ~(BLACK_BIT | WHITE_BITS);
            (*p)->marked |= g->GetCurrentWhite();
            p = &(*p)->next;
        }
        count--;
    }

    return (*p == nullptr) ? nullptr : p;
}

lu_mem BLGC::FreeObj(BLLuaState* L, GCObject* gco)
{
    switch (gco->type)
    {
    case LuaType::TSTRING:
        {
            l_mem sz = sizeof(BLTString);
            BLMem::FreeObject(L, gco, sz);
            return sz;
        }
    }
}

void BLGC::SweepStep(BLLuaState* L) {
    BLGlobalState* g = L->globalState;
    if (g->sweepgc) {
        g->sweepgc = SweepList(L, g->sweepgc, GCMAXSWEEPGCO);
        g->GCestimate = g->totalbytes + g->GCdebt;

        if (g->sweepgc) {
            return;
        }
    }
    g->gcstate = State::SweepEnd;
    g->sweepgc = NULL;
}