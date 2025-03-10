#include "blexpr.h"
#include "blopcodes.h"
#include <pplwin.h>

void BLExpr::Init(expkind k, int i)
{
    this->k = k;
    this->u.info = i;
    this->t = -1;
    this->f = -1;
}

void BLExpr::SetInt(int value)
{
    this->k = VINT;
    this->u.i = value;
}

void BLExpr::SetFloat(float value)
{
    this->k = VFLT;
    this->u.r = value;
}

void BLExpr::SetReg(int reg)
{
    this->Init(VNONRELOC, reg);
}

void BLExpr::SetRelocate(int instruction)
{
    this->Init(VRELOCATE, instruction);
}

bool BLExpr::IsNumber()
{
    return this->k == VINT || this->k == VFLT;
}

TValue BLExpr::GetValue()
{
    int ret = 0;
    TValue v;
    switch (k) {
        // 如果 v 不为空，将 v 的类型设置为整数，并将 e 的整数值赋给 v
        case VINT: {
            v.tt_ = LUA_NUMINT;
            v.value_.i = u.i;
        } break;
        // 如果 v 不为空，将 v 的类型设置为浮点数，并将 e 的浮点数值赋给 v。
        case VFLT: {
            v.tt_ = LUA_NUMFLT;
            v.value_.n = u.r;
        } break;
    default:break;
    }

    return v;
}

bool BLExpr::HasJump()
{
    return t != -1 || f != -1;
}






