#include "blcode.h"


void BLCode::UpdateJpc(BLParser* parser)
{
    FixJump(parser, parser->jpc, parser->GetPC(), -1, parser->GetPC());
    parser->jpc = -1;
}

int BLCode::CodeABC(BLParser* parser, BLOpCodes::OpCode opcode, int a, int b, int c)
{
    UpdateJpc(parser);
    BLInstruction ins = BLInstruction();
    ins.InitABC(opcode, a, b, c);
    return parser->AddInstruction(ins);
}

int BLCode::CodeABx(BLParser* parser, BLOpCodes::OpCode opcode, int a, int bx)
{
    UpdateJpc(parser);
    BLInstruction ins = BLInstruction();
    ins.InitABx(opcode, a, bx);
    return parser->AddInstruction(ins);
}

int BLCode::CodeAsBx(BLParser* parser, BLOpCodes::OpCode opcode, int a, int sbx)
{
    UpdateJpc(parser);
    BLInstruction ins = BLInstruction();
    ins.InitAsBx(opcode, a, sbx);
    return parser->AddInstruction(ins);
}


void BLCode::CodeUnOpr(BLParser* parser, int opr, BLExpr* e)
{
    BLExpr e2;
    e2.Init(VVOID, 0);
    switch (opr)
    {
    case UNOPR_MINUS: case UNOPR_BNOT:
        {
            if(e->IsNumber())
            {
                TValue v1 = e->GetValue();
                TValue v2 = e2.GetValue();
                DoMathOpr(LUA_OPT_UMN + opr, &v1, &v2);
                if (v1.tt_ == LUA_NUMINT) {
                    e->SetInt(v1.value_.i);
                } else if(v1.tt_ == LUA_NUMFLT)
                {
                    e->SetFloat(v1.value_.n);
                }
            }
            else
            {
                SaveToReg(parser, e);
                FreeReg(parser, e);

                int ins = CodeABC(parser, BLOpCodes::OpCode(BLOpCodes::OP_UNM + opr), 0, e->u.info, 0);
                e->SetRelocate(ins);
            }
            break;
        }
    case UNOPR_LEN:
        {
            SaveToReg(parser, e);
            FreeReg(parser, e);

            int ins = CodeABC(parser, BLOpCodes::OP_LEN, 0, e->u.info, 0);
            e->SetRelocate(ins);
            break;
        }
    case UNOPR_NOT:
        {
            switch (e->k)
            {
            case VFALSE: case VNIL:
                {
                    e->k = VTRUE;
                    return;
                }
            case VTRUE: case VINT: VFLT:
                {
                    e->k = VFALSE;
                    return;
                }
            }
            
            SaveToReg(parser, e);
            FreeReg(parser, e);

            int ins = CodeABC(parser, BLOpCodes::OP_NOT, 0, e->u.info, 0);
            e->SetRelocate(ins);
        }
    }
}
#define arithint(op, v1, v2) (v1->value_.i = (v1->value_.i op v2->value_.i))
#define arithnum(op, v1, v2) (v1->value_.n = (v1->value_.n op v2->value_.n))

static void intarith(int op, TValue* v1, TValue* v2) {
    switch (op) {
    case LUA_OPT_BAND:	arithint(&, v1, v2); break;
    case LUA_OPT_BOR:	arithint(|, v1, v2); break;
    case LUA_OPT_BXOR:	arithint(^, v1, v2); break;
    case LUA_OPT_BNOT:	v1->value_.i = ~v1->value_.i; break;
    case LUA_OPT_IDIV:  arithint(/, v1, v2); break;
    case LUA_OPT_SHL:	arithint(<<, v1, v2); break;
    case LUA_OPT_SHR:	arithint(>>, v1, v2); break;
    default:printf("intarith:unknow int op %c \n", op); break;
    }
}

static void numarith(int op, TValue* v1, TValue* v2) {
    switch (op) {
    case LUA_OPT_UMN: v1->value_.n = -v1->value_.n; break;
    case LUA_OPT_DIV: arithnum(/, v1, v2); break;
    case LUA_OPT_ADD: arithnum(+, v1, v2); break;
    case LUA_OPT_SUB: arithnum(-, v1, v2); break;
    case LUA_OPT_MUL: arithnum(*, v1, v2); break;
    case LUA_OPT_MOD: v1->value_.n = fmod(v1->value_.n, v2->value_.n); break;
    case LUA_OPT_POW: v1->value_.n = pow(v1->value_.n, v2->value_.n); break;
    default:printf("intarith:unknow int op %c \n", op); break;
    }
}

void BLCode::DoMathOpr(int opr, TValue* v1, TValue* v2)
{
    switch (opr)
    {
    case LUA_OPT_BAND: case LUA_OPT_BOR: case LUA_OPT_BXOR: case LUA_OPT_BNOT:
    case LUA_OPT_IDIV: case LUA_OPT_SHL: case LUA_OPT_SHR:
        {
            v1->tt_ = LUA_NUMINT;
            v2->tt_ = LUA_NUMINT;
            v1->value_.i = v1->value_.n;
            v2->value_.i = v2->value_.n;
            intarith(opr, v1, v2);

            break;
        }
    case LUA_OPT_UMN: case LUA_OPT_DIV: case LUA_OPT_ADD: case LUA_OPT_SUB:
    case LUA_OPT_MUL: case LUA_OPT_POW: case LUA_OPT_MOD:
        {
            v1->tt_ = LUA_NUMFLT;
            v2->tt_ = LUA_NUMFLT;
            v1->value_.n = v1->value_.i;
            v2->value_.n = v2->value_.i;
            numarith(opr, v1, v2);
            break;
        }  
    }
}

int BLCode::GetNextJump(BLParser* parser, int pc)
{
    BLInstruction* jmp = parser->GetInstruction(pc);
    int offset = jmp->sBx;
    if(offset == -1)
    {
        return -1;
    } else
    {
        return (pc + 1) + offset;
    }
}


void BLCode::LinkAToB(BLParser* parser, int* l1, int l2)
{
    if (l2 == -1) {
        return;
    }
    else if (*l1 == -1) {
        *l1 = l2;
    }
    else {
        int list = *l1;
        int next = 0;
        while ((next = GetNextJump(parser, list)) != -1)
            list = next;

        int offset = l2 - list - 1;
        BLInstruction* jmp = parser->GetInstruction(list);
        jmp->sBx = offset;
    }
}

void BLCode::CodeBinOpr(BLParser* parser, int opr, BLExpr* e1, BLExpr* e2)
{
    switch (opr)
    {
    case BINOPR_AND: case BINOPR_OR:
        {
            LinkAToB(parser, &e2->t, e1->t);
            LinkAToB(parser, &e2->f, e1->f);
            *e1 = *e2; // 将e2除了t和f的值赋给e1
            break;
        }
    case BINOPR_ADD: case BINOPR_SUB: case BINOPR_MUL: case BINOPR_DIV:
    case BINOPR_IDIV: case BINOPR_MOD: case BINOPR_POW: case BINOPR_BAND:
    case BINOPR_BOR: case BINOPR_BXOR: case BINOPR_SHL: case BINOPR_SHR:
        {
            bool isNum = e1->IsNumber() && e2->IsNumber();
            if(isNum)
            {
                TValue v1 = e1->GetValue();
                TValue v2 = e2->GetValue();
                DoMathOpr(LUA_OPT_ADD + opr, &v1, &v2);
                if (v1.tt_ == LUA_NUMINT) {
                    e1->SetInt(v1.value_.i);
                }
                else {
                    e1->SetFloat(v1.value_.n);
                }
            } else
            {
                SaveToReg(parser, e1);
                SaveToReg(parser, e2);

                FreeReg(parser, e1);
                FreeReg(parser, e2);

                ReserveReg(parser, 1);
                int ins = CodeABC(parser, (BLOpCodes::OpCode)(BLOpCodes::OP_ADD + opr), parser->freereg - 1, e1->u.info, e2->u.info);
                e1->SetReg(parser->freereg - 1);
            }
            break;
        }
    }
}

int BLCode::CodeJump(BLParser* parser, BLExpr* e, int b)
{
    CodeABC(parser, BLOpCodes::OP_TESTSET, 0, e->u.info, b);
    return CodeAsBx(parser, BLOpCodes::OP_JUMP, 0, -1);
}

void BLCode::CodeLogicOpr(BLParser* parser, bool isAnd, BLExpr* e)
{
    int pc;
    SaveToReg(parser, e);
    switch (e->k)
    {
    case VJMP:
        {
            break;
        }
    case VK: case VFLT: case VINT: case VTRUE:
        {
            pc = -1;
            break;
        }
    default:
        {
            FreeReg(parser, e);
            int b = 0;
            if(!isAnd)
            {
                b = 1;
            }
            pc = CodeJump(parser, e, b);
            break;
        }
    }

    if(isAnd)
    {
        // false指向最新的jump指令的位置
        LinkAToB(parser, &e->f, pc); // 将新的跳转位置与表达式 e 的假跳转列表连接
        // 
        LinkAToB(parser, &parser->jpc, e->t); // 将表达式 e 的真跳转列表更新为当前指令位置，并将 e->t 设置为 NO_JUMP。
        e->t = -1;   
    } else
    {
        LinkAToB(parser, &e->t, pc); // 将新的跳转位置与表达式 e 的假跳转列表连接
        LinkAToB(parser, &parser->jpc, e->f); // 将表达式 e 的真跳转列表更新为当前指令位置，并将 e->t 设置为 NO_JUMP。
        e->f = -1;
    }
}

void BLCode::CodeBinOprPre(BLParser* parser, int opr, BLExpr* e)
{
    switch (opr)
    {
    case BINOPR_AND:
        {
            CodeLogicOpr(parser, true, e);
            break;
        }
    case BINOPR_OR:
        {
            CodeLogicOpr(parser, false, e);
            break;
        }
    case BINOPR_ADD: case BINOPR_SUB: case BINOPR_MUL: case BINOPR_DIV:
    case BINOPR_IDIV: case BINOPR_MOD: case BINOPR_POW: case BINOPR_BAND:
    case BINOPR_BOR: case BINOPR_BXOR: case BINOPR_SHL: case BINOPR_SHR:
        {
            bool isNum = e->IsNumber();
            if(!isNum)
            {
                SaveToReg(parser, e);
            }
            break;
        }
    }
}


int BLCode::SaveToReg(BLParser* parser, BLExpr* e, bool force)
{
    switch (e->k)
    {
    case VLOCAL:
        {
            e->k = VNONRELOC;
        }
    }

    if(e->k == VNONRELOC && !force)
    {
        return e->u.info;
    }
    
    FreeReg(parser, e);
    ReserveReg(parser, 1);

    int reg = parser->freereg - 1;
    switch (e->k)
    {
    case VINT:
        {
            TValue value;
            value.tt_ = LUA_NUMINT;
            value.value_.i = e->u.i;
            CodeABx(parser, BLOpCodes::OP_LOADK, reg, parser->AddK(value));       
            break;
        }
    case VFLT:
        {
            TValue value;
            value.tt_ = LUA_NUMFLT;
            value.value_.n = e->u.r;
            CodeABx(parser, BLOpCodes::OP_LOADK, reg, parser->AddK(value));       
            break;
        }
    case VNONRELOC:
        {
            if(e->u.info != reg)
            {
                CodeABC(parser, BLOpCodes::OP_MOVE, reg, e->u.info, 0);
            }
            break;
        }
    case VRELOCATE:
        {
            BLInstruction* ins = parser->GetInstruction(e->u.info);
            ins->A = reg;
            break;
        }
    }

    if(e->HasJump())
    {
        FixJump(parser, e->f, reg, parser->GetPC(), -1);
        FixJump(parser, e->t, reg, parser->GetPC(), -1);
    }

    e->SetReg(reg);

    return reg;
}


void BLCode::FixJump(BLParser* parser, int list, int reg, int dtarget, int vtarget)
{
    int pc = parser->GetPC();
    while(list != -1)
    {
            
        int next = GetNextJump(parser, list);
        bool isTestSet = false;
        BLInstruction* ins = parser->GetInstruction(list);
        BLInstruction* prev_ins = parser->GetInstruction(list - 1);
        if(prev_ins->opCode == BLOpCodes::OP_TESTSET)
        {
            isTestSet = true;
            if(reg != -1 && reg != prev_ins->A)
            {
                prev_ins->A = reg;
            }
            else //将 OP_TESTSET 转换为 OP_TEST，仅测试寄存器值，不进行赋值
            {
            //OPTEST  
            }
        }

        if(isTestSet)
        {
            ins->sBx = dtarget - list - 1;
        }
        else
        {
            ins->sBx = vtarget - list - 1;
        }


        list = next;
    }
}

void BLCode::FreeReg(BLParser* parser, BLExpr* e)
{
    if(e->k == VNONRELOC)
    {
        if(e->u.info >= parser->activeLocNum)
        {
            parser->freereg--;
        }
        return;
    }
}

void BLCode::ReserveReg(BLParser* parser, int count)
{
    parser->freereg += count;
}


