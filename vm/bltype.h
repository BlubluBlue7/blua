#pragma once
#include <cstdint>
#include <type_traits>
namespace LuaType {
    // 基类型枚举类 (4 bits)
    enum class Base : uint8_t {
        TNUMBER         = 1,
        TLIGHTUSERDATA  = 2,
        TBOOLEAN        = 3,
        TSTRING         = 4,
        TNIL            = 5,
        TTABLE          = 6,
        TFUNCTION       = 7,
        TTHREAD         = 8,
        TNONE           = 9
    };

    // 数值子类型 (4 bits)
    enum class NumberSub : uint8_t { INT = 0, FLOAT = 1 };

    // 函数子类型
    enum class FunctionSub : uint8_t { LCL = 0, LCF = 1, CCL = 2 };

    // 字符串子类型
    enum class StringSub : uint8_t { LONG = 0, SHORT = 1 };

    // 类型组合工具
    class Type {
    public:
        constexpr Type(Base base) : value(static_cast<uint8_t>(base)) {}
        
        template<typename Sub>
        constexpr Type(Base base, Sub sub) : 
            value(static_cast<uint8_t>(base) | 
                 (static_cast<uint8_t>(sub) << 4)) {}

        constexpr Base base() const { 
            return static_cast<Base>(value & 0x0F); 
        }

        template<typename Sub>
        constexpr Sub sub() const {
            return static_cast<Sub>((value & 0xF0) >> 4);
        }

        constexpr operator uint8_t() const { return value; }

    private:
        uint8_t value;
    };

    // 预定义类型常量
    constexpr Type TNUMBER        { Base::TNUMBER };
    constexpr Type TLIGHTUSERDATA{ Base::TLIGHTUSERDATA };
    constexpr Type TBOOLEAN      { Base::TBOOLEAN };
    constexpr Type TSTRING        { Base::TSTRING };
    constexpr Type TNIL           { Base::TNIL };
    constexpr Type TTABLE         { Base::TTABLE };
    constexpr Type TFUNCTION      { Base::TFUNCTION };
    constexpr Type TTHREAD        { Base::TTHREAD };
    constexpr Type TNONE          { Base::TNONE };

    constexpr Type NUMINT  { Base::TNUMBER, NumberSub::INT };
    constexpr Type NUMFLT  { Base::TNUMBER, NumberSub::FLOAT };

    constexpr Type TLCL    { Base::TFUNCTION, FunctionSub::LCL };
    constexpr Type TLCF    { Base::TFUNCTION, FunctionSub::LCF };
    constexpr Type TCCL    { Base::TFUNCTION, FunctionSub::CCL };

    constexpr Type LNGSTR  { Base::TSTRING, StringSub::LONG };
    constexpr Type SHRSTR  { Base::TSTRING, StringSub::SHORT };

    // 类型检查工具函数
    constexpr bool is_number(Type t) { 
        return t.base() == Base::TNUMBER; 
    }

    template<typename Sub>
    constexpr bool is_subtype(Type t) {
        return (t.base() == Base::TNUMBER && 
               std::is_same_v<Sub, NumberSub>) ||
               (t.base() == Base::TFUNCTION && 
               std::is_same_v<Sub, FunctionSub>) ||
               (t.base() == Base::TSTRING && 
               std::is_same_v<Sub, StringSub>);
    }
} // namespace LuaType