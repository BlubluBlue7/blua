#include "blopcodes.h"
#include <iostream>

void BLInstruction::InitABC(BLOpCodes::OpCode code, int a, int b, int c)
{
    mode = 0;
    opCode = code;
    A = a;
    B = b;
    C = c;
}


void BLInstruction::InitABx(BLOpCodes::OpCode code, int a, int bx)
{
    mode = 1;
    opCode = code;
    A = a;
    Bx = bx;
}

void BLInstruction::InitAsBx(BLOpCodes::OpCode code, int a, int sbx)
{
    mode = 2;
    opCode = code;
    A = a;
    sBx = sbx;
}


void BLInstruction::Print()
{
    if(mode == 1)
    {
        std::cout << BLOpCodes::opCodeToString[opCode] << " " << A << " " << Bx << std::endl;
    }
    else if(mode == 0)
    {
        std::cout << BLOpCodes::opCodeToString[opCode] << " " << A << " " << B << " " << C << std::endl;
    }
    else
    {
        std::cout << BLOpCodes::opCodeToString[opCode] << " " << A << " " << sBx << std::endl;
    }
}

