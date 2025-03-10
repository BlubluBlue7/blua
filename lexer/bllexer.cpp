#include "bllexer.h"
#include <unordered_map>
void BLLexer::inclinenumber() {
    // TODO: 实现行号统计
}

// 判断是否为十六进制数字字符
bool BLLexer::isHexDigit(int c) {
    return std::isdigit(c) ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}


std::string BLLexer::GetStringFromBuffer()
{
    std::string str = buffer;
    buffer.clear();
    return str;
}

// C++ 风格的关键字字典
const std::unordered_map<std::string, TokenType> LUA_KEYWORDS = {
    {"local",    TokenType::TK_LOCAL},
    {"nil",      TokenType::TK_NIL},
    {"true",     TokenType::TK_TRUE},
    {"false",    TokenType::TK_FALSE},
    {"end",      TokenType::TK_END},
    {"then",     TokenType::TK_THEN},
    {"if",       TokenType::TK_IF},
    {"elseif",   TokenType::TK_ELSEIF},
    {"else",     TokenType::TK_ELSE},
    {"not",      TokenType::TK_NOT},
    {"and",      TokenType::TK_AND},
    {"or",       TokenType::TK_OR},
    {"do",       TokenType::TK_DO},
    {"for",      TokenType::TK_FOR},
    {"in",       TokenType::TK_IN},
    {"while",    TokenType::TK_WHILE},
    {"repeat",   TokenType::TK_REPEAT},
    {"until",    TokenType::TK_UNTIL},
    {"break",    TokenType::TK_BREAK},
    {"return",   TokenType::TK_RETURN},
    {"function", TokenType::TK_FUNCTION}
};
BLLexer::BLLexer() : current(0)
{
    // // TString* env = luaS_newliteral(L, LUA_ENV);
    // // luaC_fix(L, obj2gco(env));
    //
    // for (int i = 0; i < NUM_RESERVED; i++) {
    //     std::string reserver = luaX_tokens[i];
    //     // TString* reserved = luaS_newliteral(L, luaX_tokens[i]);
    //     // luaC_fix(L, obj2gco(reserved));
    //     // reserved->extra = i + FIRST_REVERSED;
    // }
}

bool BLLexer::ReadLuaFile(const std::string& fileName) {
    if (!reader.ReadLuaFile(fileName)) {
        std::cerr << "无法打开文件: " << fileName << std::endl;
        return false;
    }
    GetNextChar();
    return true;
}

std::string BLLexer::GetTokenValue() const {
    // 根据 token 类型返回字符串表示（这里使用 std::visit 也可以）
    switch (tokenCurrent.type) {
    case TokenType::TK_FLOAT:
        return std::to_string(std::get<double>(tokenCurrent.value));
    case TokenType::TK_INT:
        return std::to_string(std::get<long long>(tokenCurrent.value));
    case TokenType::TK_STRING:
    case TokenType::TK_NAME:
        return std::get<std::string>(tokenCurrent.value);
    default:
        return std::to_string(static_cast<int>(tokenCurrent.type));
    }
}

int BLLexer::GetTokenValueInt() const
{
    if(tokenCurrent.type == TokenType::TK_INT)
    {
        return std::get<long long>(tokenCurrent.value);
    }
    else
    {
        return 0;
    }
}

float BLLexer::GetTokenValueFloat() const
{
    if(tokenCurrent.type == TokenType::TK_FLOAT)
    {
        return std::get<double>(tokenCurrent.value);
    }
    else
    {
        return 0;
    }
}

const TokenValue& BLLexer::GetTokenValueObject() const {
    return tokenCurrent.value;
}

TokenType BLLexer::GetTokenType() const {
    return tokenCurrent.type;
}

void BLLexer::GetNextChar() {
    current = reader.GetChar();
}

void BLLexer::SaveToBuffer(char c) {
    buffer.push_back(c);
}

int BLLexer::ReadString(int delimiter) {
    // 跳过起始定界符
    GetNextChar();
    while (current != delimiter) {
        if (current == '\n' || current == '\r' || current == '\0') {
            // TODO: 处理未闭合字符串错误
            return static_cast<int>(TokenType::TK_STRING);
        }
        if (current == '\\') {
            GetNextChar();
            int c = 0;
            switch (current) {
            case 't':  c = '\t'; break;
            case 'v':  c = '\v'; break;
            case 'a':  c = '\a'; break;
            case 'b':  c = '\b'; break;
            case 'f':  c = '\f'; break;
            case 'n':  c = '\n'; break;
            case 'r':  c = '\r'; break;
            default:
                // 未识别的转义字符：直接保存 '\' 与当前字符
                SaveToBuffer('\\');
                c = current;
                break;
            }
            SaveToBuffer(static_cast<char>(c));
            GetNextChar();
        }
        else {
            SaveToBuffer(static_cast<char>(current));
            GetNextChar();
        }
    }
    // 跳过结束定界符
    GetNextChar();
    return static_cast<int>(TokenType::TK_STRING);
}

int BLLexer::Str2number(bool hasDot) {
    // 如果以 . 开头，提前补充 "0."
    if (hasDot) {
        SaveToBuffer('0');
        SaveToBuffer('.');
    }
    while (std::isdigit(current) || current == '.') {
        if (current == '.') {
            if (hasDot) {
                // 多个小数点，格式错误
                // TODO: 错误处理
                return 0;
            }
            hasDot = true;
        }
        SaveToBuffer(static_cast<char>(current));
        GetNextChar();
    }
    // 将当前非数字字符也保存（便于后续处理，可根据需要调整）
    // SaveToBuffer(static_cast<char>(current));

    if (hasDot) {
        double num = std::atof(buffer.c_str());
        token.value = num;
        token.type = TokenType::TK_FLOAT;
    }
    else {
        long long num = std::atoll(buffer.c_str());
        token.value = num;
        token.type = TokenType::TK_INT;
    }
    buffer.clear();
    return static_cast<int>(token.type);
}

int BLLexer::Str2hex() {
    int count = 0;
    while (isHexDigit(current)) {
        SaveToBuffer(static_cast<char>(current));
        GetNextChar();
        ++count;
    }
    // 保存当前非十六进制字符
    SaveToBuffer(static_cast<char>(current));
    if (count <= 0) {
        // TODO: 错误处理：没有合法的十六进制数字
    }
    long long num = std::strtoll(buffer.c_str(), nullptr, 0);
    token.value = num;
    token.type = TokenType::TK_INT;
    buffer.clear();
    return static_cast<int>(token.type);
}

bool BLLexer::IsNewLine() const {
    return current == '\n' || current == '\r';
}

TokenType BLLexer::GetToken() {
    // 重置 token 类型
    token.type = TokenType::TK_EOS;
    while (true) {
        switch (current) {
            // 空白与换行处理
            case '\n':
            case '\r':
                inclinenumber();
                GetNextChar();
                continue;
            case ' ':
            case '\t':
            case '\v':
                GetNextChar();
                continue;
            case '\0':
                GetNextChar();
                token.type = TokenType::TK_EOS;
                return token.type;
            // 处理 '-'（减号或注释）
            case '-': {
                GetNextChar();
                if (current == '-') {  // 注释
                    while (!IsNewLine() && current != '\0')
                        GetNextChar();
                    continue;
                }
                else {
                    token.type = static_cast<TokenType>('-');
                    return token.type;
                }
            }
            // 处理单字符运算符：+ * /
            case '+':
            case '*':
            case '/': {
                int c = current;
                GetNextChar();
                token.type = static_cast<TokenType>(c);
                return token.type;
            }
            // 处理 '~'（可能为 "!="）
            case '~': {
                GetNextChar();
                if (current == '=') {
                    GetNextChar();
                    token.type = TokenType::TK_NOTEQUAL;
                }
                else {
                    token.type = static_cast<TokenType>('~');
                }
                return token.type;
            }
            case '%': {
                GetNextChar();
                token.type = TokenType::TK_MOD;
                return token.type;
            }
            case '>': {
                GetNextChar();
                if (current == '=') {
                    GetNextChar();
                    token.type = TokenType::TK_GREATEREQUAL;
                }
                else if (current == '>') {
                    GetNextChar();
                    token.type = TokenType::TK_SHR;
                }
                else {
                    token.type = static_cast<TokenType>('>');
                }
                return token.type;
            }
            case '<': {
                GetNextChar();
                if (current == '=') {
                    GetNextChar();
                    token.type = TokenType::TK_LESSEQUAL;
                }
                else if (current == '<') {
                    GetNextChar();
                    token.type = TokenType::TK_SHL;
                }
                else {
                    token.type = static_cast<TokenType>('<');
                }
                return token.type;
            }
            case '=': {
                GetNextChar();
                if (current == '=') {
                    GetNextChar();
                    token.type = TokenType::TK_EQUAL;
                }
                else {
                    token.type = static_cast<TokenType>('=');
                }
                return token.type;
            }
            // 字符串字面量（支持单引号和双引号）
            case '"':
            case '\'': {
                int delim = current;
                token.type = static_cast<TokenType>(ReadString(delim));
                token.value = GetStringFromBuffer();
                return token.type;
            }
            // 数字字面量（包括十进制和十六进制）
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                buffer.clear();
                if (current == '0') {
                    SaveToBuffer(static_cast<char>(current));
                    GetNextChar();
                    if (current == 'x' || current == 'X') {
                        SaveToBuffer(static_cast<char>(current));
                        GetNextChar();
                        token.type = static_cast<TokenType>(Str2hex());
                    }
                    else {
                        token.type = static_cast<TokenType>(Str2number(false));
                    }
                }
                else {
                    token.type = static_cast<TokenType>(Str2number(false));
                }
                return token.type;
            }
            // 处理 '.'：可能是数字、连接符或者单独的点
            case '.': {
                GetNextChar();
                if (std::isdigit(current)) {
                    token.type = static_cast<TokenType>(Str2number(true));
                }
                else if (current == '.') {
                    GetNextChar();
                    if (current == '.') {
                        GetNextChar();
                        token.type = TokenType::TK_VARARG;
                    }
                    else {
                        token.type = TokenType::TK_CONCAT;
                    }
                }
                else {
                    token.type = static_cast<TokenType>('.');
                }
                return token.type;
            }
            // 单字符分隔符
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}':
            case ',': {
                char c = static_cast<char>(current);
                GetNextChar();
                token.type = static_cast<TokenType>(c);
                return token.type;
            }
            // 标识符（由字母、数字、下划线组成）
            default: {
                if (std::isalpha(current) || current == '_') {
                    buffer.clear();
                    while (std::isalnum(current) || current == '_') {
                        SaveToBuffer(static_cast<char>(current));
                        GetNextChar();
                    }
                    std::string str = GetStringFromBuffer();
                    token.type = TokenType::TK_NAME;
                    auto it = LUA_KEYWORDS.find(str);
                    if (it != LUA_KEYWORDS.end()) {
                        token.type = it->second;  // 返回关键字类型
                    }
                    token.value = str;
                    return token.type;
                }
                else {
                    // 未知字符，直接返回其 ASCII 码
                    int c = current;
                    GetNextChar();
                    token.type = static_cast<TokenType>(c);
                    return token.type;
                }
            }
        } // end switch
    } // end while
}

TokenType BLLexer::GetNextToken() {
    if(tokenAhead.type != TokenType::TK_EOS)
    {
        tokenCurrent = tokenAhead;
        tokenAhead.type = TokenType::TK_EOS;
        return tokenCurrent.type;
    }

    GetToken();
    tokenCurrent = token;
    return tokenCurrent.type;
}

TokenType BLLexer::LookAhead() {
    GetToken();
    tokenAhead = token;
    return tokenAhead.type;
}

//
// int BLLexer::GetToken()
// {
//     token.type = -1;
//     while(token.type == -1)
//     {
//         switch (current)
//         {
//         // 空白 换行
//         case '\n':
//         case '\r':
//             {
//                 inclinenumber();
//                 GetNextChar();
//                 break;    
//             }
//         case ' ':
//         case '\t': // 水平制表符
//         case '\v': // 垂直制表符
//             {
//                 GetNextChar();
//                 break;
//             }
//         case '\0':
//             {
//                 GetNextChar();
//                 token.type = (int)TokenType::TK_EOS;
//                 break;   
//             }
//         // 运算符
//         case '-':
//             {
//                 GetNextChar();
//                 // 判断是注释
//                 bool isComment = current == '-';
//                 if(isComment)
//                 {
//                     while(!IsNewLine() && current != '\0')
//                     {
//                         GetNextChar();
//                     }
//                 }
//                 else
//                 {   
//                     token.type = '-';
//                 }
//                 break;   
//             }
//         case '+': case '*': case '/':
//             {
//                 int c = current;
//                 GetNextChar();
//                 token.type = c;
//                 break;   
//             }
//         case '~':
//             {
//                 GetNextChar();
//                 bool isNotEqual = current == '=';
//                 if(isNotEqual)
//                 {
//                     GetNextChar();
//                     token.type = (int)TokenType::TK_NOTEQUAL;
//                 }
//                 else
//                 {
//                     token.type = '~';
//                 }
//                 break;   
//             }
//         case '%':
//             {
//                 GetNextChar();
//                 token.type = (int)TokenType::TK_MOD;
//                 break;   
//             }
//         case '>':
//             {
//                 GetNextChar();
//                 if(current == '=')
//                 {
//                     GetNextChar();
//                     token.type = (int)TokenType::TK_GREATEREQUAL;
//                 }
//                 else if(current == '>')
//                 {
//                     GetNextChar();
//                     token.type = (int)TokenType::TK_SHR;
//                 }
//                 else
//                 {
//                     token.type = '>';
//                 }
//                 break;   
//             }
//         case '<':
//             {
//                 GetNextChar();
//                 if(current == '=')
//                 {
//                     GetNextChar();
//                     token.type = (int)TokenType::TK_LESSEQUAL;
//                 }
//                 else if(current == '<')
//                 {
//                     GetNextChar();
//                     token.type = (int)TokenType::TK_SHL;
//                 }
//                 else
//                 {
//                     token.type = '<';
//                 }
//                 break;   
//             }
//         case '=':
//             {
//                 GetNextChar();
//                 if(current == '=')
//                 {
//                     GetNextChar();
//                     token.type = (int)TokenType::TK_EQUAL;
//                 }
//                 else
//                 {
//                     token.type = '=';
//                 }
//                 break;   
//             }
//         // 字面量
//         case '"': case '\'':
//             {
//                 token.type = ReadString(current);
//                 token.value->s = GetStringFromBuffer();
//                 break;   
//             }
//         case '0': case '1': case '2': case '3': case '4':
//         case '5': case '6': case '7': case '8': case '9':
//             {
//                 if(current == '0')
//                 {
//                     SaveToBuffer(current);
//                     GetNextChar();
//                     if(current == 'x' || current == 'X')
//                     {
//                         SaveToBuffer(current);
//                         GetNextChar();
//                         token.type = Str2hex();
//                     } else
//                     {
//                         token.type = Str2number(false);
//                     }
//                 }
//                 else
//                 {
//                     token.type = Str2number(false);
//                 }
//                 break;
//             }
//         case '.':
//             {
//                 GetNextChar();
//                 if (isdigit(current))
//                 {
//                     token.type = Str2number(true);
//                     //token.value = 
//                 }
//                 else if (current == '.')
//                 {
//                     GetNextChar();
//                     if(current == '.')
//                     {
//                         GetNextChar();
//                         token.type = (int)TokenType::TK_VARARG;
//                     }
//                     else
//                     {
//                         token.type = (int)TokenType::TK_CONCAT;
//                     }
//                 }
//                 else
//                 {
//                     token.type = '.';
//                 }
//                 break;   
//             }
//         case '(':
//         case ')':
//         case '[':
//         case ']':
//         case '{':
//         case '}':
//             {
//                 char c = current;
//                 GetNextChar();
//                 token.type = c;
//                 break;   
//             }
//         
//         case ',':
//             {
//                 GetNextChar();
//                 token.type = ',';
//                 break;
//             }
//         default:
//             {
//                 if(isalpha(current) || current == '_')
//                 {
//                     while(isalpha(current) || current == '_' || isdigit(current))
//                     {
//                         SaveToBuffer(current);
//                         GetNextChar();
//                     }
//
//                     // saveToBuffer(current);
//
//                     token.type = (int)TokenType::TK_NAME;
//                     token.value->s = GetStringFromBuffer();
//                 }
//                 else
//                 {
//                     int c = current;
//                     GetNextChar();
//                     token.type = c;
//                 }
//                 break;
//             }
//         }
//     }
//     
//     return token.type;
// }
