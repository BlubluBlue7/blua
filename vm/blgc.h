#pragma once
#include <type_traits>

#include "bltable.h"
#include "blua.h"

class GCObject;
class BLLuaState;
class BLTString;
class BLTValue;
enum class State : int {
    Pause = 0,
    Propagate = 1,
    Atomic = 2,
    InsideAtomic = 3,
    SweepAllGC = 4,
    SweepEnd = 5
};

enum class Flag : unsigned {
    White0 = 0, // 1
    White1 = 1, // 10
    Black = 2   // 100
};

constexpr unsigned bit_mask(Flag bit) noexcept {
    return 1U << static_cast<unsigned>(bit);
}
constexpr unsigned bit_combine(Flag b1, Flag b2) noexcept {
    return bit_mask(b1) | bit_mask(b2);
}
// 颜色组合常量
constexpr unsigned WHITE_BITS = bit_combine(Flag::White0, Flag::White1); // 11
constexpr unsigned BLACK_BIT = bit_mask(Flag::Black); // 100
constexpr unsigned GCMAXSWEEPGCO = 25;
// GC 对象标记操作工具类
class GCHelper {
public:
    // 重置/设置/测试位标记
    template<typename T>
    static void reset_bits(T& value, unsigned mask) noexcept {
        value &= static_cast<T>(~mask);
    }

    template<typename T>
    static void set_bits(T& value, unsigned mask) noexcept {
        value |= static_cast<T>(mask);
    }

    template<typename T>
    static bool test_bits(T value, unsigned mask) noexcept {
        return (value & mask) != 0;
    }

    // 颜色判断函数
    template<typename GCObject>
    static bool is_white(const GCObject* obj) noexcept {
        return test_bits(obj->marked, WHITE_BITS);
    }

    template<typename GCObject>
    static bool is_gray(const GCObject* obj) noexcept {
        return !test_bits(obj->marked, BLACK_BIT | WHITE_BITS);
    }

    template<typename GCObject>
    static bool is_black(const GCObject* obj) noexcept {
        return test_bits(obj->marked, BLACK_BIT);
    }

    static bool is_dead(lu_byte ow, lu_byte marked) noexcept {
        return (!((marked ^ WHITE_BITS) & (ow)));
    }

    // 对象标记操作
    template<typename L, typename T>
    static void mark_object(L* lua_state, T* obj) {
        if (is_white(obj)) {
            really_mark_object(lua_state, to_gc(obj));
        }
    }


};

// 类型检查函数（使用模板特化）
template<typename T>
constexpr bool is_collectable() noexcept {
    return std::is_same_v<T, BLLuaState> ||
           std::is_same_v<T, BLTString> ||
           std::is_same_v<T, BLTable>;
           // std::is_same_v<T, LClosure> ||
           // std::is_same_v<T, CClosure>;
}

template<typename T>
bool is_collectable(const T* obj) noexcept {
    return is_collectable<T>();
}

class GCObject
{
public:
    GCObject* next;
    lu_byte type;
    lu_byte marked;

    virtual ~GCObject() = default;
    static GCObject* NewObj(BLLuaState* L, int type, size_t size);
    void ChangeWhite();
};

class BLGC
{
public:
    // GC 触发条件封装
    static void CheckGC(BLLuaState* L);

    static void StepGC(BLLuaState* L);
    static l_mem SingleStep(BLLuaState* L);
    static void MarkObject(BLLuaState* lua_state, GCObject* obj);
    static void MarkValue(BLLuaState* lua_state, BLTValue* value);

    static void PropagateAll(BLLuaState* L);
    static void PropagateMark(BLLuaState* L);
    static void Atomic(BLLuaState* L);
    static GCObject** SweepList(BLLuaState* L, GCObject** p, size_t count);
    static void SweepStep(BLLuaState* L);

    static lu_mem FreeObj(BLLuaState* L, GCObject* gco);
};
