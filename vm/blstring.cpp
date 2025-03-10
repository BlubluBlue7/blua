#include "blstring.h"
#include <cstring>

#include "blmem.h"
#include "bltype.h"


BLTString* BLTString::NewString(BLLuaState* L, const char* data)
{
    size_t len = strlen(data);
    if(len <= MAXSHORTSTR)
    {
        return InternalStr(L, data, len);
    }
    else
    {
        return CreateLongStr(L, data, len);
    }
}

int BLTString::HashLngStr(BLLuaState* L)
{
    if(extra == 0)
    {
        hash = GetHash(L, data, u.lnglen, L->globalState->seed);
        extra = 1;
    }

    return hash;
}

unsigned BLTString::GetHash(BLLuaState* L, const char* str, unsigned int l, unsigned int h)
{
    h = h ^ l;
    unsigned int step = (l >> 5) + 1;
    for (int i = 0; i < l; i = i + step) {
        h ^= (h << 5) + (h >> 2) + (lu_byte)str[i];
    }
    return h;
}

BLTString* BLTString::InternalStr(BLLuaState* L, const char* data, size_t len)
{
    BLGlobalState* g = L->globalState;
    StringTable* tb = &g->strt;
    unsigned int h = GetHash(L, data, len, g->seed);
    BLTString** list = &tb->hash[h & (tb->size - 1)];

    for (BLTString* ts = *list; ts; ts = ts->u.hnext) {
        if (ts->shrlen == len && (memcmp(ts->data, data, len * sizeof(char)) == 0)) {
            if (GCHelper::is_dead(g->GetOtherWhite(), ts->marked)) {
                ts->ChangeWhite();
            }
            return ts;
        }
    }

    if (tb->nuse >= tb->size && tb->size < INT_MAX / 2) {
        Resize(L, tb->size * 2);
        list = &tb->hash[h & (tb->size - 1)];
    }

    BLTString* ts = CreateStr(L, data, len, LuaType::SHRSTR, h);
    ts->u.hnext = *list;
    *list = ts;
    tb->nuse++;

    return ts;
}

unsigned int BLTString::Resize(BLLuaState* L, unsigned int nsize)
{
    BLGlobalState* g = L->globalState;
    unsigned int osize = g->strt.size;
    if (nsize > osize) {
        g->strt.hash.resize(nsize);
        // luaM_reallocvector(L, g->strt.hash, osize, nsize, TString*);
        for (int i = osize; i < nsize; i ++) {
            g->strt.hash[i] = nullptr;
        }
    }

    // all TString value will be rehash by nsize
    for (int i = 0; i < g->strt.size; i ++) {
        BLTString* ts = g->strt.hash[i];
        g->strt.hash[i] = NULL;

        while(ts) {
            BLTString* old_next = ts->u.hnext;
            unsigned int hash = ts->hash & (nsize - 1);
            ts->u.hnext = g->strt.hash[hash];
            g->strt.hash[hash] = ts;
            ts = old_next;
        }
    }

    // shrink string hash table 
    if (nsize < osize) {
        g->strt.hash.resize(nsize);
        // lua_assert(g->strt.hash[nsize] == NULL && g->strt.hash[osize - 1] == NULL);
        // luaM_reallocvector(L, g->strt.hash, osize, nsize, TString*);
    }
    g->strt.size = nsize;

    return g->strt.size;
}


size_t BLTString::GetLStringSize(size_t l)
{
    return sizeof(BLTString) + (l + 1) * sizeof(char);
}
BLTString* BLTString::CreateLongStr(BLLuaState* L, const char* data, size_t len)
{
    return CreateStr(L, data, len, LuaType::LNGSTR, L->globalState->seed);
}

BLTString* BLTString::CreateStr(BLLuaState* L, const char* data, size_t len, LuaType::Type type, unsigned int hash)
{
    size_t total_size = GetLStringSize(len);

    BLTString* str = BLMem::NewString(L, type);

    str->data = new char[len + 1];
    std::copy(data, data + len, str->data);
    str->data[len] = '\0';
    str->extra = 0;

    if (type == LuaType::SHRSTR) {
        str->shrlen = len;
        str->hash = hash;
        str->u.hnext = NULL;
    }
    else if (type == LuaType::LNGSTR) {
        str->hash = 0;
        str->u.lnglen = len;
    }

    
    return str;
}

