#pragma once
#include "blparser.h"
#include "blopcodes.h"

class BLCode
{
public:
    static int CodeABC(BLParser* parser, BLOpCodes::OpCode opcode, int a, int b, int c);
    static int CodeABx(BLParser* parser, BLOpCodes::OpCode opcode, int a, int bx);
    static int CodeAsBx(BLParser* parser, BLOpCodes::OpCode opcode, int a, int sbx);
    
    static void CodeUnOpr(BLParser* parser, int opr, BLExpr* e);
    static void CodeBinOprPre(BLParser* parser, int opr, BLExpr* e);
    static void CodeBinOpr(BLParser* parser, int opr, BLExpr* e1, BLExpr* e2);
    static void CodeLogicOpr(BLParser* parser, bool isAnd, BLExpr* e);
    static int CodeJump(BLParser* parser, BLExpr* e, int b);

    static void DoMathOpr(int opr, TValue* v1, TValue* v2);

    static int SaveToReg(BLParser* parser, BLExpr* e, bool force = false);
    static void FreeReg(BLParser* parser, BLExpr* e);
    static void ReserveReg(BLParser* parser, int count);

    static void LinkAToB(BLParser* parser, int* l1, int l2);
    static int GetNextJump(BLParser* parser, int pc);
    static void UpdateJpc(BLParser* parser);
    static void FixJump(BLParser* parser, int list, int reg, int dtarget, int vtarget);
};
