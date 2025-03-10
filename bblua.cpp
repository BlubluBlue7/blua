#include <iostream>

#include "lexer/bllexer.h"
#include "parser/blparser.h"
#include "vm/blmem.h"
#include "vm/blstate.h"
#include "vm/bltype.h"
#include "vm/blstring.h"

static int test_main01(struct BLLuaState* L) {
    lua_Integer i = L->GetIntFromStack(-1);
    printf("test_main01 luaL_tointeger value = %ld\n", i);
    return 0;
}

void test_cp1()
{
    BLLuaState* ls = BLMem::NewLuaState();
    ls->AddCFuncToStack(&test_main01);
    ls->AddIntToStack(1);
    ls->PCall(ls, 1, 0);


    BLGlobalState::CloseLuaState();
}

void test_cp2()
{
    BLLuaState* ls = BLMem::NewLuaState();
    for(int i = 0; i < 5; ++i)
    {
        ls->AddNilToStack();
    }

    int start_time = time(NULL);
    int end_time = time(NULL);
    size_t max_bytes = 0;
    BLGlobalState* g = ls->globalState;
    for(int j = 0; j < 50000000000; ++j)
    {
        BLTValue* o = ls->GetTValueFromStackByAddr((j % 5) + 1);
        BLTString* ts = BLMem::NewString(ls, LuaType::SHRSTR);
        o->value.gc = ts;
        o->type = LuaType::TSTRING;

        BLGC::CheckGC(ls);
        
        if((g->totalbytes + g->GCdebt) >max_bytes)
        {
            max_bytes = g->totalbytes + g->GCdebt;
        }

        if(j % 1000 == 0)
        {
            std::cout << "timestamp:" << time(NULL) << " totalbytes:" << (float)(g->totalbytes + g->GCdebt) / 1024.0f << " kb" << std::endl;
        }
    }

    end_time = time(NULL);
    std::cout << "finish test start_time:" << start_time << " end_time:" << end_time << " max_bytes:" << (float)max_bytes / 1024.0f << " kb" << std::endl;
    BLGlobalState::CloseLuaState();
}

const char* g_shrstr = "This is a short string";
const char* g_lngstr = "This is a long string. This is a long string. This is a long string. This is a long string.";

int print(BLLuaState* L) {
    char* str = L->GetStringFromStack(-1);
    printf("%s\n", str);
    return 0;
}

void test_print(BLLuaState* L, const char* str) {
    L->AddCFuncToStack(print);
    L->AddStringToStack(str);
    L->PCall(L, 1, 0);
}

void test_cp3()
{
    BLLuaState* ls = BLMem::NewLuaState();
    test_print(ls, g_shrstr);
    test_print(ls, g_lngstr);


    BLGlobalState::CloseLuaState();
}

void test_kv(BLLuaState* L)
{
    int tbl_idx = L->GetStackSize();
    printf("stack_size:%d \n", tbl_idx);

    // push integer key
    L->AddIntToStack(1);
    L->AddStringToStack("test integer key");
    

}

void test_cp4()
{
    BLLuaState* ls = BLMem::NewLuaState();
    BLTable* table = BLMem::NewTable(ls);
    ls->AddTableToStack(table);
    test_kv(ls);
    ls->Pop();
    
    


    
    BLGlobalState::CloseLuaState();
}
int main()
{
    test_cp3();
    // BLLexer lexer;
    // lexer.ReadLuaFile("lua/test.lua");
    // BLParser parser(&lexer);
    // parser.Parse();
    // parser.Print();
}
