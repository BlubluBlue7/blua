#pragma once

// basic object type
#define LUA_TNUMBER 1
#define LUA_TLIGHTUSERDATA 2
#define LUA_TBOOLEAN 3
#define LUA_TSTRING 4
#define LUA_TNIL 5
#define LUA_TTABLE 6
#define LUA_TFUNCTION 7
#define LUA_TTHREAD 8
#define LUA_TPROTO 9
#define LUA_TNONE 10
#define LUA_NUMS LUA_TNONE 
#define LUA_TDEADKEY (LUA_NUMS + 1)

// lua number type 
#define LUA_NUMINT (LUA_TNUMBER | (0 << 4))
#define LUA_NUMFLT (LUA_TNUMBER | (1 << 4))

// lua function type 
#define LUA_TLCL (LUA_TFUNCTION | (0 << 4))
#define LUA_TLCF (LUA_TFUNCTION | (1 << 4))
#define LUA_TCCL (LUA_TFUNCTION | (2 << 4))

// string type 
#define LUA_LNGSTR (LUA_TSTRING | (0 << 4))
#define LUA_SHRSTR (LUA_TSTRING | (1 << 4))

#define SIZE_OP 6
#define SIZE_A  8
#define SIZE_B  9
#define SIZE_C  9
#define POS_A SIZE_OP
#define POS_B (SIZE_C + SIZE_A + SIZE_OP)
#define POS_C (SIZE_A + SIZE_OP)

#define GET_OPCODE(i) (i & 0x3F)
#define GET_ARG_A(i) ((i & 0x3FC0) >> POS_A)
#define GET_ARG_B(i) ((i & 0xFF800000) >> POS_B)
#define GET_ARG_C(i) ((i & 0x7FC000) >> POS_C)
#define GET_ARG_Bx(i) ((i & 0xFFFFC000) >> (SIZE_A + SIZE_OP))
#define GET_ARG_sBx(i) (GET_ARG_Bx(i) - LUA_IBIAS)

#define LUA_OPT_UMN		1
#define LUA_OPT_LEN		2
#define LUA_OPT_BNOT	3
#define LUA_OPT_NOT		4

#define	LUA_OPT_ADD		5
#define	LUA_OPT_SUB		6
#define	LUA_OPT_MUL		7
#define	LUA_OPT_DIV		8
#define LUA_OPT_IDIV    9
#define	LUA_OPT_MOD		10
#define	LUA_OPT_POW		11
#define	LUA_OPT_BAND    12
#define	LUA_OPT_BOR		13
#define	LUA_OPT_BXOR	14
#define	LUA_OPT_SHL		15
#define	LUA_OPT_SHR		16
#define	LUA_OPT_CONCAT  17
#define	LUA_OPT_GREATER 18
#define	LUA_OPT_LESS	19
#define	LUA_OPT_EQ		20
#define	LUA_OPT_GREATEQ 21
#define	LUA_OPT_LESSEQ  22
#define	LUA_OPT_NOTEQ	23
#define	LUA_OPT_AND		24
#define	LUA_OPT_OR		25
#include <map>
#include <string>

class BLOpCodes
{
public:
	enum OpMode {
		iABC,
		iABx,
		iAsBx,
	};

	enum OpArgMask {
		OpArgU,     // argument is used, and it's value is in instruction
		OpArgR,     // argument is used, and it's value is in register, the position of register is indexed by instruction
		OpArgN,     // argument is not used
		OpArgK,     // argument is used, and it's value is in constant table, the index is in instruction
	};
		
    enum OpCode {
		OP_MOVE,        // A, B; R[A] = R[B] load a value from a register to another register;
		OP_LOADK,       // A, B; R[A] = RK[B]; load a value from k to register
		OP_GETUPVAL,    // A, B; R[A] = UpValue[B]
		OP_CALL,        // A, B, C; R[A], ... R[A + C - 2] = R(A)(R[A + 1], ... , R[A + B - 1])
						// A index the function in the stack. 
						// B represents the number of params, if B is 1, the function has no parameters, else if B is greater than 1, the function has B - 1 parameters, and if B is 0
						// it means that the function parameters range from A+1 to the top of stack
						// C represents the number of return, if C is 1, there is no value return, else if C is greater than 1, then it has C - 1 return values, or if C is 0
						// the return values range from A to the top of stack
		OP_RETURN,      // A, B; return R[A], ... R[A + B - 2]
						// return the values to the calling function, B represent the number of results. if B is 1, that means no value return, if B is greater than 1, it means 
						// there are B - 1 values return. And finally, if B is 0, the set of values range from R[A] to the top of stack, are return to the calling function
		OP_GETTABUP,   // A, B, C; R[A] = Upval[B][RK[C]]
		OP_GETTABLE,    // A, B, C; R[A] = R[B][RK[C]]

		OP_SELF,        // A, B, C; R(A+1) = R(B); R(A) = R(B)[RK(C)];

		OP_TEST,		// A, B, C; if not (R(A) <=> C) then pc++];
		OP_TESTSET,		// A B C   if (R(B) <=> C) then R(A) := R(B) else pc++ 
		OP_JUMP,		// A sBx   pc+=sBx; if (A) close all upvalues >= R(A - 1)

		/*
		codeunexpval
		*/
		OP_UNM,			// A B     R(A) := -R(B)
		OP_LEN,			// A B     R(A) := length of R(B)
		OP_BNOT,		// A B     R(A) := ~R(B)
		/*
		codenot
		*/
		OP_NOT,			// A B     R(A) := not R(B)
		/*
		luaK_posfix
		codebinexp
		*/
		OP_ADD,			// A B C   R(A) := RK(B) + RK(C)
		OP_SUB,			// A B C   R(A) := RK(B) - RK(C)
		OP_MUL,			// A B C   R(A) := RK(B) * RK(C)
		OP_DIV,			// A B C   R(A) := RK(B) / RK(C)
		OP_IDIV,		// A B C   R(A) := RK(B) // RK(C)
		OP_MOD,			// A B C   R(A) := RK(B) % RK(C)
		OP_POW,			// A B C   R(A) := RK(B) ^ RK(C)
		OP_BAND,		// A B C   R(A) := RK(B) & RK(C)
		OP_BOR,			// A B C   R(A) := RK(B) | RK(C)
		OP_BXOR,		// A B C   R(A) := RK(B) ~ RK(C)
		OP_SHL,			// A B C   R(A) := RK(B) << RK(C)
		OP_SHR,			// A B C   R(A) := RK(B) >> RK(C)
		OP_CONCAT,		// A B C   R(A) := R(B).. ... ..R(C)
		/*
		codecmp
		*/
		OP_EQ,			// A B C   if ((RK(B) == RK(C)) ~= A) then pc++
		OP_LT,			// A B C   if ((RK(B) <  RK(C)) ~= A) then pc++
		OP_LE,			// A B C   if ((RK(B) <= RK(C)) ~= A) then pc++

		OP_LOADBOOL,    // A B C   R(A) := (Bool)B; if(C) PC++;
		OP_LOADNIL,     // A B     R(A) := ... := R(B) := nil

		OP_SETUPVAL,    // A B     UpValue[B] := R(A)
		OP_SETTABUP,    // A B C   UpValue[A][RK(B)] := RK(C)
		OP_NEWTABLE,    // A B C   R(A) := {} (size = B,C)
		OP_SETLIST,     // A B C   R(A)[(C-1)*FPF+i] := R(A+i), 1 <= i <= B
		OP_SETTABLE,    // A B C   R(A)[RK(B)] = RK(C)

		OP_FORPREP,     // A sBx   FORPREP A sBx R(A) -= R(A+2); PC += sBx
		OP_FORLOOP,     // A sBx   FORLOOP A sBx R(A) += R(A+2)
						//         if R(A) < ? = R(A + 1) then {
						//	          PC += sBx; R(A + 3) = R(A)
						//         }
		OP_TFORCALL,    // A C	R(A+3), ... ,R(A+2+C) := R(A)(R(A+1), R(A+2));
		OP_TFORLOOP,    // A sBx	if R(A+1) ~= nil then { R(A)=R(A+1); pc += sBx }
		OP_CLOSURE,     // A Bx	R(A) := closure(KPROTO[Bx])
		NUM_OPCODES,
	};

public:
	inline  static std::map<OpCode, std::string> opCodeToString = {
		{OP_MOVE, "OP_MOVE"},
		{OP_LOADK, "OP_LOADK"},
		{OP_GETUPVAL, "OP_GETUPVAL"},
		{OP_CALL, "OP_CALL"},
		{OP_RETURN, "OP_RETURN"},
		{OP_GETTABUP, "OP_GETTABUP"},
		{OP_GETTABLE, "OP_GETTABLE"},
		{OP_SELF, "OP_SELF"},
		{OP_TEST, "OP_TEST"},
		{OP_TESTSET, "OP_TESTSET"},
		{OP_JUMP, "OP_JUMP"},
		{OP_UNM, "OP_UNM"},
		{OP_LEN, "OP_LEN"},
		{OP_BNOT, "OP_BNOT"},
		{OP_NOT, "OP_NOT"},
		{OP_ADD, "OP_ADD"},
		{OP_SUB, "OP_SUB"},
		{OP_MUL, "OP_MUL"},
		{OP_DIV, "OP_DIV"},
		{OP_IDIV, "OP_IDIV"},
		{OP_MOD, "OP_MOD"},
		{OP_POW, "OP_POW"},
		{OP_BAND, "OP_BAND"},
		{OP_BOR, "OP_BOR"},
		{OP_BXOR, "OP_BXOR"},
		{OP_SHL, "OP_SHL"},
		{OP_SHR, "OP_SHR"},
		{OP_CONCAT, "OP_CONCAT"},
		{OP_EQ, "OP_EQ"},
		{OP_LT, "OP_LT"},
		{OP_LE, "OP_LE"},
		{OP_LOADBOOL, "OP_LOADBOOL"},
		{OP_LOADNIL, "OP_LOADNIL"},
		{OP_SETUPVAL, "OP_SETUPVAL"},
		{OP_SETTABUP, "OP_SETTABUP"},
		{OP_NEWTABLE, "OP_NEWTABLE"},
		{OP_SETLIST, "OP_SETLIST"},
		{OP_SETTABLE, "OP_SETTABLE"},
		{OP_FORPREP, "OP_FORPREP"},
		{OP_FORLOOP, "OP_FORLOOP"},
		{OP_TFORCALL, "OP_TFORCALL"},
		{OP_TFORLOOP, "OP_TFORLOOP"},
		{OP_CLOSURE, "OP_CLOSURE"},
		{NUM_OPCODES, "ERROR"},
	};
	
};

class BLInstruction
{
public:
	BLOpCodes::OpCode opCode;
	int A;
	int B;
	int C;
	int Bx;
	int sBx;
	int mode;
	void InitABC(BLOpCodes::OpCode code, int a, int b, int c);
	void InitABx(BLOpCodes::OpCode code, int a, int bx);
	void InitAsBx(BLOpCodes::OpCode code, int a, int sbx);
	
	void Print();
};