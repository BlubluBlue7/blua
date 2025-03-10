#pragma once
typedef enum expkind {
    VVOID,			// expression is void
    VNIL,			// expression is nil value
    VFLT,			// expression is float value
    VINT,			// expression is integer value
    VTRUE,			// expression is true value
    VFALSE,			// expression is false value
    VCALL,			// expression is a function call, info field of struct expdesc is represent instruction pc

    VLOCAL,			// expression is a local value, info field of struct expdesc is represent the pos of the stack
    VUPVAL,			// expression is a upvalue, ind is in use
    VINDEXED,		// ind field of struct expdesc is in use

    VK,				// expression is a constant, info field of struct expdesc is represent the index of k
    VJMP,
    VRELOCATE,		// expression can put result in any register, info field represents the instruction pc
    VNONRELOC,		// expression has result in a register, info field represents the pos of the stack
} expkind;

union Value {
    struct GCObject* gc;
    void* p;
    int b;
    long long i;
    double n;
};

class TValue
{
public:
    Value value_;
    int tt_;
};
class BLExpr
{
public:
    expkind k;
    int t = -1;				// patch list of 'exit when true'
    int f = -1;				// patch list of 'exit when false'
    union {
        int info;			
        long long i;		// for VINT
        double r;		// for VFLT

        struct {
            int t;		// the index of the upvalue or table
            int vt;		// whether 't' is a upvalue(VUPVAL) or table(VLOCAL)
            int idx;	// index (R/K)
        } ind;
    } u;

    void Init(expkind k, int i);
    void SetInt(int value);
    void SetFloat(float value);
    void SetReg(int reg);
    void SetRelocate(int instruction);

    bool IsNumber();
    TValue GetValue();
    bool HasJump();
};
