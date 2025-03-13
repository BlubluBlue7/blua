#include "bltable.h"

#include "blluastate.h"
#include "math.h"
#include "../parser/blopcodes.h"

int luaO_ceillog2(int value) {
    int i = 0;

    for (; i < 32; i++) {
        if ((int)pow(2, i) >= value)
            return i;
    }

    return i;
}

#define lua_numbertointeger(n, p) \
    (n >= (lua_Number)INT_MIN) && \
    (n <= (lua_Number)INT_MAX) && \
    ((*p = (lua_Integer)n), 1)

static int l_hashfloat(lua_Number n) {
    int i = 0;
    lua_Integer ni = 0;

    // 使用frexp(n, &i)将浮点数n分解为尾数（在[0.5, 1)范围内）和指数i。例如，n = 3.0会被分解为尾数0.75和指数2。
    // 将尾数乘以-INT_MIN（假设为32位系统，INT_MIN为-2,147,483,648），使
    // 尾数扩展到较大的整数范围。例如，0.75 * 2,147,483,648 = 1,610,612,736。
    n = frexp(n, &i) * - (lua_Number)INT_MIN;
    if (!lua_numbertointeger(n, &ni)) {
        // lua_assert(lua_numisnan(n) || fabs(n) == (lua_Number)HUGE_VAL));
        return 0;
    }
    else {
        unsigned int u = (unsigned int)ni + (unsigned int)i;
        return u <= INT_MAX ? u : ~u;
    }
}

void BLTable::SetNodeSize(BLLuaState* L, int size)
{
    if (size == 0)
    {
        nodeSize = 0;
        // node = &defaultNode;
        lastFree = 0;
    } else
    {
        int realSize = luaO_ceillog2(size);
        nodeSize = realSize;
        // if(nodeSize > (sizeof(int) * CHAR_BIT - 1))
        // {
        //     
        // }

        int node_size = 1 << realSize;
        node.resize(node_size);
        // node = new Node[node_size];
        lastFree = node_size;
        for (int i = 0; i < node_size - 1; i++)
        {
            Node* n = node[i];
            BLTKey* k = &n->key;
            k->nk.next = 0;
            n->key.tvk.type = LuaType::TNIL;
            n->val.type = LuaType::TNIL;
        }
    }
}

BLTValue* BLTable::GetNodeKey(Node* n)
{
    return &n->key.tvk;
}

BLTValue* BLTable::GetNodeValue(Node* n)
{
    return &n->val;
}

Node* BLTable::GetNodeByIndex(int index)
{
    return node[index];
}

Node* BLTable::GetNode(BLLuaState* L, class BLTValue* key)
{
    switch (key->type)
    {
    case LuaType::NUMINT:
        {
            return hashInt(key->value.i);
        }
    case LuaType::NUMFLT:
        {
            return hashInt(l_hashfloat(key->value.n));
        }
    case LuaType::TBOOLEAN:
        {
            return hashBool(key->value.b);
        }
    case LuaType::SHRSTR:
        {
            return hashStr((BLTString*)key->value.gc);
        }
    case LuaType::LNGSTR:
        {
            BLTString* str = (BLTString*)key->value.gc;
            str->HashLngStr(L);
            return hashStr(str);
        }
    case LuaType::TLIGHTUSERDATA:
        {
            return hashPointer(key->value.p);
        }
    case LuaType::TLCF:
        {
            return hashPointer(key->value.f);
        }
    default:
        {
            return hashPointer(key->value.gc);
        }
    }
}


Node* BLTable::hashInt(int key)
{
    return GetNodeByIndex(key & ((1 << nodeSize) - 1));
}

Node* BLTable::hashBool(bool key)
{
    return GetNodeByIndex(key & ((1 << nodeSize) - 1));
}

Node* BLTable::hashStr(BLTString* str)
{
    return GetNodeByIndex(str->hash & ((1 << nodeSize) - 1));
}

Node* BLTable::hashPointer(void* p)
{
    int t = ((unsigned int)((size_t)p & UINT_MAX));
    return GetNodeByIndex(t & ((1 << nodeSize) - 1));
}

BLTValue* BLTable::GetValue(BLLuaState* L, BLTValue* key)
{
    switch(key->type)
    {
        case LuaType::TNIL: return &luaO_nilobject;
        case LuaType::NUMINT: return GetInt(L, key->value.i);
        case LuaType::NUMFLT: return GetInt(L, l_hashfloat(key->value.n));
        case LuaType::SHRSTR:
            {
                BLTString* str = (BLTString*)key->value.gc;
                return GetInt(L, str->hash);

            }
        case LuaType::LNGSTR:
            {
                BLTString* str = (BLTString*)key->value.gc;
                return GetInt(L, str->hash);
            }
    }
}

BLTValue* BLTable::GetInt(BLLuaState* L, int key)
{
    if(key - 1 < arraySize)
    {
        return array[key - 1];
    } else
    {
        Node* n = hashInt(key);
        while(true)
        {
            BLTKey* k = &n->key;
            if(k->tvk.IsInteger() && k->tvk.value.i == key)
            {
                return &n->val;
            } else 
            {
                int next = k->nk.next;
                if(next == 0)
                {
                    break;
                }
                n += next;
            }
        }
    }

    return &luaO_nilobject;
}

BLTValue* BLTable::GetStr(BLLuaState* L, BLTString* str)
{
    if(str->type == LuaType::SHRSTR)
    {
        Node* n = hashStr(str);
        while(true)
        {
            BLTKey* k = &n->key;
            BLTString* keyStr = (BLTString*)k->tvk.value.gc;
            if(k->tvk.IsShrStr() && str->EqualShrStr(keyStr))
            {
                return &n->val;
            } else
            {
                int next = k->nk.next;
                if(next == 0)
                {
                    break;
                }
                n += next;
            }
        }
    }
    else
    {
        BLTValue k;
        k.type = LuaType::LNGSTR;
        k.value.gc = str;
        return GetGeneric(L, &k);
    }

    return nullptr;
}

BLTValue* BLTable::GetGeneric(BLLuaState* L, BLTValue* key)
{
    Node* n = GetNode(L, key);
    while (true)
    {
        BLTValue* k = GetNodeKey(n);
        if(k->EqualTo(key))
        {
            return GetNodeValue(n);
        }
        else
        {
            int next = n->key.nk.next;
            if(next == 0)
            {
                break;
            }
            n += next;
        }
    }

    return &luaO_nilobject;
}

int BLTable::SetInt(BLLuaState* L, int key, const BLTValue* value)
{
    const BLTValue* p = GetInt(L, key);
    BLTValue* cell = nullptr;
    if(p != luaO_nilobject)
    {
        cell = (BLTValue*)p;
    }
    else
    {
        BLTValue k;
        k.type = LuaType::NUMINT;
        k.value.i = key;
        cell = NewKey(L, k);
    }

    cell->type = value->type;
    cell->value = value->value;
    return 1;
}

BLTValue* BLTable::SetValue(BLLuaState* L, const BLTValue* key)
{
    const BLTValue* p = GetValue(key);
    if(p != luaO_nilobject)
    {
        return (BLTValue*)p;
    }
    else
    {
        return NewKey(L, key);
    }
}


// typedef struct Auxnode {
//     struct Table* t;
//     unsigned int size;
// } Auxnode;
//
// static int aux_set_node_size(struct lua_State* L, void* ud) {
//     struct Auxnode* n = cast(struct Auxnode*, ud);
//     setnodesize(L, n->t, n->size);
//     return 0;
// }
