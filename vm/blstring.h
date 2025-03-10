#pragma once
#include <vector>
#include <cvt/wstring>

#include "blgc.h"
#include "bltype.h"

class BLTString;
class StringTable
{
public:
    std::vector<BLTString*> hash;
    unsigned int nuse;
    unsigned int size;
};

class BLTString: public GCObject
{
public:
    unsigned int hash;
    unsigned short extra;
    unsigned short shrlen;
    char* data;
    union
    {
        BLTString* hnext;
        size_t lnglen;
    } u;

    int HashLngStr(BLLuaState* L);
    static unsigned int Resize(BLLuaState* L, unsigned int nsize);
    static unsigned int GetHash(BLLuaState* L, const char* str, unsigned int l, unsigned int h);
    static BLTString* NewString(BLLuaState* L, const char* data);
    static constexpr size_t MAXSHORTSTR = 40;

    static BLTString* InternalStr(BLLuaState* L, const char* data, size_t len);
    static BLTString* CreateLongStr(BLLuaState* L, const char* data, size_t len);
    static BLTString* CreateStr(BLLuaState* L, const char* data, size_t len, LuaType::Type type, unsigned int hash);
    static size_t GetLStringSize(size_t l);
};
