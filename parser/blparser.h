#pragma once
#include <list>
#include <stack>

#include "blexpr.h"
#include "bllocvar.h"
#include "blopcodes.h"
#include "../lexer/bllexer.h"

#define UNOPR_PRIORITY 12
typedef enum UnOpr {
    UNOPR_MINUS,
    UNOPR_LEN,
    UNOPR_BNOT,
    UNOPR_NOT,

    NOUNOPR,
} UnOpr;

typedef enum BinOpr {
    BINOPR_ADD = 0,
    BINOPR_SUB,
    BINOPR_MUL,
    BINOPR_DIV,
    BINOPR_IDIV,
    BINOPR_MOD,
    BINOPR_POW,
    BINOPR_BAND,
    BINOPR_BOR,
    BINOPR_BXOR,
    BINOPR_SHL,
    BINOPR_SHR,
    BINOPR_CONCAT,
    BINOPR_GREATER,
    BINOPR_LESS,
    BINOPR_EQ,
    BINOPR_GREATEQ,
    BINOPR_LESSEQ,
    BINOPR_NOTEQ,
    BINOPR_AND,
    BINOPR_OR,

    NOBINOPR,
} BinOpr;

class BLFuncState
{
    
};



class BLParser
{
public:
    BLParser(BLLexer* lex);
    void Parse();
    void Print();
    int freereg;
    int AddInstruction(BLInstruction instruction);
    BLInstruction* GetInstruction(int index);
    int GetPC();
    
    int AddK(TValue k);
    int GetLocNum();
    int activeLocNum = 0;

    int jpc = -1;
private:
    BLLexer* lexer;

    std::vector<BLLocVar> locList;
    std::vector<BLInstruction> instructionList;
    std::vector<TValue> kList;
    void ParseStatement();
    void ParseLocalStat();
    void ParseLocalVarStat();

    int ParseExprList(BLExpr* e);
    int ParseExpr(BLExpr* e, int limit = 0);
    void ParseSimpleExpr(BLExpr* e);
    void ParseSuffixedExpr(BLExpr* e);
    void ParsePrimaryExpr(BLExpr* e);
    
    UnOpr ParseUnOpr();
    BinOpr ParseBinOpr();

    void ParseVar(BLExpr* e, std::string varName);
    void ParseVarInContext(BLExpr* e, std::string varName);
    
    void NewLocalVar(std::string varName);
    int SearchVar(std::string varName);
    

    void DoAssign(BLExpr* e, int nvars, int nexps);
};
