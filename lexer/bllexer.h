#pragma once
#include <string>
#include <variant>
#include <vector>
#include <cstddef> // for size_t
#include "blreader.h"
// #include "../parser/blparser.h"

// Token 起始值，便于与 ASCII 值区分
constexpr int TOKEN_START_POS = 257;

// 使用 C++11 的 enum class 定义 Token 类型，保证作用域清晰
enum class TokenType : int {
    // 关键字
    TK_LOCAL = TOKEN_START_POS,
    TK_NIL,
    TK_TRUE,
    TK_FALSE,
    TK_END,
    TK_THEN,
    TK_IF,
    TK_ELSEIF,
    TK_ELSE,
    TK_NOT,
    TK_AND,
    TK_OR,
    TK_DO,
    TK_FOR,
    TK_IN,
    TK_WHILE,
    TK_REPEAT,
    TK_UNTIL,
    TK_BREAK,
    TK_RETURN,
    TK_FUNCTION,
    
    // 标识符和字面量
    TK_STRING,
    TK_NAME,
    TK_FLOAT,
    TK_INT,
    
    // 运算符
    TK_NOTEQUAL,    // !=
    TK_EQUAL,       // ==
    TK_GREATEREQUAL,// >=
    TK_LESSEQUAL,   // <=
    TK_SHL,         // <<
    TK_SHR,         // >>
    TK_MOD,         // %
    TK_DOT,         // .
    TK_VARARG,      // ...
    TK_CONCAT,      // ..
    
    // 文件结束标志
    TK_EOS,         
};

constexpr int NUM_RESERVED = (int)TokenType::TK_FUNCTION - TOKEN_START_POS + 1;
// 使用 std::variant 来表示 Token 的可能取值（这里加入了 std::monostate 表示无值状态）
using TokenValue = std::variant<std::monostate, long long, double, std::string>;

class BLToken {
public:
    TokenType type{ TokenType::TK_EOS };
    TokenValue value; // 直接作为成员，无需动态分配

    BLToken() = default;
    // 根据需要可添加其他构造函数或赋值操作符
};

class BLLexer {
public:
    BLLexer();
    
    // 打开指定的 Lua 文件，准备词法分析
    bool ReadLuaFile(const std::string& fileName);
    

    
    // 获取当前 token 的值（如字符串形式），如果不存在则返回空串
    std::string GetTokenValue() const;
    int GetTokenValueInt() const;
    float GetTokenValueFloat() const;
    
    // 获取当前 token 类型的整型值（主要用于调试或后续处理）
    TokenType GetTokenType() const;
    
    // 获取当前 token 的 TokenValue 对象
    const TokenValue& GetTokenValueObject() const;

    TokenType GetNextToken();
    TokenType LookAhead();
private:
    BLReader reader;
    BLToken token;
    BLToken tokenCurrent;
    BLToken tokenAhead;
    std::string buffer;
    size_t current = 0; // 当前在 buffer 中的位置
    
    // 以下为内部辅助函数
    
    // 获取下一个 token，并返回其类型
    TokenType GetToken();
    void GetNextChar();
    bool IsNewLine() const;
    void SaveToBuffer(char c);
    int Str2number(bool hasDot);
    int Str2hex();
    int ReadString(int delimiter);
    std::string GetStringFromBuffer();

    static void inclinenumber();
    static bool isHexDigit(int c);
};
