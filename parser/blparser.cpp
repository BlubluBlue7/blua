#include "blparser.h"

#include "blcode.h"
#include "../lexer/bllexer.h"

BLParser::BLParser(BLLexer* lex)
{
    lexer = lex;
}

void BLParser::Parse()
{
    freereg = 0;
    ParseStatement();
}

void BLParser::ParseStatement()
{
    lexer->GetNextToken();
    while (true)
    {
        TokenType tokenType = lexer->GetTokenType();
        switch (tokenType)
        {
        case TokenType::TK_LOCAL:
            ParseLocalStat();
            break;
        case TokenType::TK_EOS:
            return;
        }
    }
}

void BLParser::ParseLocalStat()
{
    TokenType tokenType = lexer->GetNextToken();
    if(tokenType == TokenType::TK_FUNCTION)
    {
        
    }
    else
    {
        ParseLocalVarStat();
    }
}

void BLParser::ParseLocalVarStat()
{
    // local a = b
    // 变量
    int nvars = 0;
    while(true)
    {
        nvars++;
        NewLocalVar(lexer->GetTokenValue());
        bool isEnd = lexer->GetNextToken() != (TokenType)',';
        if(!isEnd)
        {
            lexer->GetNextToken();
        }
        else
        {
            break;
        }
    }

    if(lexer->GetTokenType() == (TokenType)'=')
    {
        BLExpr e;
        e.Init(VVOID, 0);
        int nexps = ParseExprList(&e);
        // 赋值
        DoAssign(&e, nvars, nexps);
    }
    else
    {
        // 赋值nil
        // DoAssign(nvars, 0);
    }

    activeLocNum++;
}

void BLParser::DoAssign(BLExpr* e, int nvars, int nexps)
{
    int extra = nvars - nexps;
    if(e->k == VCALL)
    {
        
    }
    else
    {
        if(e->k != VVOID)
        {
            BLCode::SaveToReg(this, e, true);
        }
        if(extra > 0)
        {
            
        }
    }

    if (nexps > nvars)
    {
        
    }
}

void BLParser::NewLocalVar(std::string varName)
{
    BLLocVar var = BLLocVar(varName);
    locList.push_back(var);
}

int BLParser::ParseExprList(BLExpr* e)
{
    lexer->GetNextToken();
    int var = 1;
    ParseExpr(e);
    
    while(true)
    {
        if(lexer->GetTokenType() == (TokenType)',')
        {
            lexer->GetNextToken();
            ParseExpr(e);
            var++;
        } else if(lexer->GetTokenType() == (TokenType)';')
        {
            break;
        }
        else
        {
            break;
        }
    }

    return var;
}

static const struct {
    int left;   // left priority for each binary operator
    int right;  // right priority
} priority[] = {
    {10,10}, {10,10},		   // '+' and '-'
    {11,11}, {11,11}, {11,11}, {11, 11}, // '*', '/', '//' and '%'
    {14,13},				   // '^' right associative
    {6,6}, {4,4}, {5,5},	   // '&', '|' and '~'
    {7,7}, {7,7},			   // '<<' and '>>'
    {9,8},					   // '..' right associative
    {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, // '>', '<', '>=', '<=', '==', '~=',
    {2,2}, {1,1},			   // 'and' and 'or'
};

int BLParser::ParseExpr(BLExpr* e, int limit)
{
    int unopr = ParseUnOpr();
    if(unopr != NOUNOPR)
    {
        lexer->GetNextToken();
        ParseExpr(e, UNOPR_PRIORITY);
        BLCode::CodeUnOpr(this, unopr, e);
    }
    else
    {
        ParseSimpleExpr(e);
        lexer->GetNextToken();
    }

    int binopr = ParseBinOpr();
    while(binopr != NOBINOPR && priority[binopr].left > limit)
    {
        BLExpr e2;
        e2.Init(VVOID, 0);

        lexer->GetNextToken();
        BLCode::CodeBinOprPre(this, binopr, e);
        int nextop = ParseExpr(&e2, priority[binopr].right);
        BLCode::CodeBinOpr(this, binopr, e, &e2);

        binopr = nextop;
    }

    return binopr;
}

void BLParser::ParseSimpleExpr(BLExpr* e)
{
    switch (lexer->GetTokenType())
    {
    case TokenType::TK_STRING:
        {
            
        }
    case TokenType::TK_INT:
        {
            e->SetInt(lexer->GetTokenValueInt());
            break;
        }
    case TokenType::TK_FLOAT:
        {
            e->SetFloat(lexer->GetTokenValueFloat());
            break;
        }
    case TokenType::TK_TRUE:
        {
            e->Init(VTRUE, 0);
            break;
        }
    case TokenType::TK_FALSE:
        {
            e->Init(VFALSE, 0);
            break;
        }
    case TokenType::TK_NIL:
        {
            e->Init(VNIL, 0);
            break;
        }
    case (TokenType)'{':
        {
            break;
        }
    case TokenType::TK_FUNCTION:
        {
            break;
        }
    default:
        {
            ParseSuffixedExpr(e);
            break;
        }
    }
}

void BLParser::ParseSuffixedExpr(BLExpr* e)
{
    ParsePrimaryExpr(e);
}

void BLParser::ParsePrimaryExpr(BLExpr* e)
{
    switch (lexer->GetTokenType())
    {
    case TokenType::TK_NAME:
        ParseVar(e, lexer->GetTokenValue());
        break;
    case (TokenType)'(':
        break;
    }
}

void BLParser::ParseVar(BLExpr* e, std::string varName)
{
    // 
    ParseVarInContext(e, varName);
    // global
    if(e->k == VVOID)
    {
        ParseVarInContext(e, varName);
    }
}

void BLParser::ParseVarInContext(BLExpr* e, std::string varName)
{
    int reg = SearchVar(varName);
    if(reg >= 0)
    {
        e->Init(VLOCAL, reg);
    } else
    {
        
    }
}

int BLParser::SearchVar(std::string varName)
{
    // 遍历 locList
    for(int i = 0; i < locList.size(); ++i)
    {
        BLLocVar locVar = locList[i];
        if(locVar.name == varName)
        {
            return i;
        }
    }

    return -1;
}




UnOpr BLParser::ParseUnOpr()
{
    switch (lexer->GetTokenType()) {
    case (TokenType)'-': return UNOPR_MINUS;
    case TokenType::TK_NOT: return UNOPR_NOT;
    case (TokenType)'#': return UNOPR_LEN;
    case (TokenType)'~': return UNOPR_BNOT;
    default: return NOUNOPR;
    }
}

BinOpr BLParser::ParseBinOpr()
{
    switch (lexer->GetTokenType()) {
    case (TokenType)'+': return BINOPR_ADD;
    case (TokenType)'-': return BINOPR_SUB;
    case (TokenType)'*': return BINOPR_MUL;
    case (TokenType)'/': return BINOPR_DIV;
    case TokenType::TK_MOD: return BINOPR_MOD;
    case (TokenType)'^': return BINOPR_POW;
    case (TokenType)'&': return BINOPR_BAND;
    case (TokenType)'|': return BINOPR_BOR;
    case (TokenType)'~': return BINOPR_BXOR;
    case TokenType::TK_SHL: return BINOPR_SHL;
    case TokenType::TK_SHR: return BINOPR_SHR;
    case TokenType::TK_CONCAT: return BINOPR_CONCAT;
    case (TokenType)'>': return BINOPR_GREATER;
    case (TokenType)'<': return BINOPR_LESS;
    case TokenType::TK_GREATEREQUAL: return BINOPR_GREATEQ;
    case TokenType::TK_LESSEQUAL: return BINOPR_LESSEQ;
    case TokenType::TK_EQUAL: return BINOPR_EQ;
    case TokenType::TK_NOTEQUAL: return BINOPR_NOTEQ;
    case TokenType::TK_AND: return BINOPR_AND;
    case TokenType::TK_OR: return BINOPR_OR;
    default:return NOBINOPR;
    }
}

int BLParser::AddInstruction(BLInstruction instruction)
{
    instructionList.push_back(instruction);
    return instructionList.size() - 1;
}

BLInstruction* BLParser::GetInstruction(int index)
{
    return &instructionList[index];
}

int BLParser::GetPC()
{
    return instructionList.size();
}

void BLParser::Print()
{
    for(int i = 0; i < kList.size(); ++i)
    {
        TValue k = kList[i];
        if(k.tt_ == LUA_NUMINT)
        {
            std::cout << "k[" << i << "] = " << k.value_.i << std::endl;
        }
        else if(k.tt_ == LUA_NUMFLT)
        {
            std::cout << "k[" << i << "] = " << k.value_.n << std::endl;
        }
    }
    for(int i = 0; i < instructionList.size(); ++i)
    {
        BLInstruction instruction = instructionList[i];
        instruction.Print();
    }
}

int BLParser::AddK(TValue k)
{
    kList.push_back(k);
    return kList.size() - 1;
}

int BLParser::GetLocNum()
{
    return locList.size();
}








