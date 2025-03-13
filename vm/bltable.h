#pragma once
#include <vector>

#include "blgc.h"
#include "blobject.h"
#include "blstring.h"
#include "../parser/blexpr.h"

// 假设必要的基础类型和函数声明已存在（如lua_State, TValue, Table等）
class BLTable;
inline bool ttistable(const BLTValue* t) {
    // 假设这是检查TValue是否为表类型的实现
    return t->type == LuaType::TTABLE;
}

inline bool ttisnil(const BLTValue* t) {
    // 检查TValue是否为nil
    return t->type == LuaType::TNIL;
}

inline BLTable* hvalue(const BLTValue* t) {
    // 将TValue*转换为对应的Table指针
    return reinterpret_cast<BLTable*>(t->value.gc);
}

inline void setobj(BLTValue* dest, const BLTValue* src) {
    // 值复制操作，假设TValue是POD类型
    *dest = *src;
}

// 快速获取模板
template<typename GetFunc>
inline bool luaV_fastget(BLLuaState* L, const BLTValue* t, const BLTValue* k, GetFunc get, const BLTValue*& slot) {
    if (!ttistable(t)) {
        slot = nullptr;
        return false;
    }
    BLTable* tbl = hvalue(t);
    slot = get(L, tbl, k);
    return !ttisnil(slot);
}

// 快速设置模板
template<typename GetFunc>
inline bool luaV_fastset(BLLuaState* L, BLTValue* t, const BLTValue* k, BLTValue* v, GetFunc get, BLTValue*& slot) {
    if (!ttistable(t)) {
        slot = nullptr;
        return false;
    }
    BLTable* tbl = hvalue(t);
    slot = get(L, tbl, k);
    if (ttisnil(slot)) {
        return false;
    }
    setobj(slot, v);
    // luaC_barrierback(L, tbl, slot);
    return true;
}

// // GetTable操作
// inline void luaV_gettable(BLLuaState* L, const TValue* t, const TValue* k, TValue* val) {
//     const TValue* slot = nullptr;
//     if (luaV_fastget(L, t, k, luaH_get, slot)) {
//         setobj(val, slot);
//     } else {
//         luaV_finishget(L, hvalue(t), val, slot);
//     }
// }
//
// // SetTable操作
// inline void luaV_settable(BLLuaState* L, TValue* t, const TValue* k, TValue* val) {
//     TValue* slot = nullptr;
//     if (!luaV_fastset(L, t, k, val, luaH_get, slot)) {
//         luaV_finishset(L, hvalue(t), k, val, slot);
//     }
// }

// 以下函数需要在其他地方实现
void luaV_finishget(BLLuaState* L, BLTable* t, TValue* val, const TValue* slot);
void luaV_finishset(BLLuaState* L, BLTable* t, const TValue* key, TValue* val, const TValue* slot);

class BLTKey
{
public:
    BLTValue tvk;
    struct my_struct
    {
        Value v;
        int tt_;
        int next;
    } nk;
};

class Node
{
public:
    BLTKey key;
    BLTValue val;
};

extern Node defaultNode;
class BLTable: public GCObject
{
public:
    BLTValue* GetNodeKey(Node* node);
    BLTValue* GetNodeValue(Node* node);
    Node* GetNodeByIndex(int index);
    Node* GetNode(BLLuaState* L, BLTValue* key);
    Node* hashInt(int key);
    Node* hashBool(bool key);
    Node* hashStr(BLTString* str);
    Node* hashPointer(void* p);
    
    std::vector<BLTValue*> array;
    unsigned int arraySize;
    std::vector<Node*> node;
    unsigned int nodeSize;
    int64_t lastFree;
    GCObject* gclist;

    void SetNodeSize(BLLuaState* L, int size);
    
    BLTValue* GetValue(BLLuaState* L, BLTValue* key);
    BLTValue* GetInt(BLLuaState* L, int key);
    BLTValue* GetStr(BLLuaState* L, BLTString* str);
    BLTValue* GetGeneric(BLLuaState* L, BLTValue* key);

    BLTValue* SetValue(BLLuaState* L);
    int SetInt(BLLuaState* L, int key, const BLTValue* value);

    BLTValue* NewKey(BLLuaState* L);
};
