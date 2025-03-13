#include "blmem.h"
#include "bltype.h"
#include "blstring.h"

BLLuaState* BLMem::NewLuaState()
{
    BLGlobalState* g;
    BLLuaState* l;

    LG* lg = new LG();
    BLGlobalState::lg = lg;
    g = &lg->g;
    g->gcstate = State::Pause;
    g->currentwhite = bit_mask(Flag::White0);
    g->totalbytes = sizeof(LG);
    g->allgc = NULL;
    g->sweepgc = NULL;
    g->gray = NULL;
    g->grayagain = NULL;
    g->GCdebt = 0;
    g->GCmemtrav = 0;
    g->GCestimate = 0;
    g->GCstepmul = BLGlobalState::GCSTEPMUL;
    
    l = &lg->lx.l;
    l->globalState = g;
    l->Init();
    
    g->mainThread = l;

    g->InitStr();

    return l;
}

void BLMem::CloseLuaState(LG* lg)
{
    delete lg;
}

BLTString* BLMem::NewString(BLLuaState* L, LuaType::Type type)
{
    BLTString* ts = new BLTString();

    BLGlobalState* g = L->globalState;
    g->UpdateDebt(sizeof(BLTString));

    ts->marked = g->GetCurrentWhite();
    ts->next = g->allgc;
    ts->type = type;
    g->allgc = ts;
    return ts;
}


BLTable* BLMem::NewTable(BLLuaState* L)
{
    BLTable* table = new BLTable();
    // table->array = nullptr;
    table->arraySize = 0;
    // table->node = nullptr;
    table->nodeSize = 0;
    table->lastFree = 0;
    table->gclist = nullptr;

    table->SetNodeSize(L, 0);

    BLGlobalState* g = L->globalState;
    g->UpdateDebt(sizeof(BLTable));

    return table;
}

void BLMem::FreeTable()
{
    
}

void BLMem::FreeObject(BLLuaState* L, GCObject* o, l_mem size)
{
    delete o;

    BLGlobalState* g = L->globalState;
    g->UpdateDebt(- size);
}

