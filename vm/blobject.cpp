#include "blobject.h"
#include "blstring.h"

bool BLTValue::EqualTo(BLTValue* other)
{
    if(IsNumber() && IsNan() || other->IsNumber() && other->IsNan())
    {
        return false;
    }

    if(type != other->type)
    {
        if(IsNumber() && other->IsNumber())
        {
            double fa = IsInteger() ? value.i : value.n;
            double fb = other->IsInteger() ? other->value.i : other->value.n;
            return fa == fb;
        }
        else
        {
            return false;
        }
    }

    switch (type)
    {
    case LuaType::TNIL:
        {
            return true;
        }
    case LuaType::NUMFLT:
        {
            return value.n == other->value.n;
        }
    case LuaType::NUMINT:
        {
            return value.i == other->value.i;
        }
    case LuaType::TBOOLEAN:
        {
            return value.b == other->value.b;
        }
    case LuaType::SHRSTR:
        {
            BLTString* str = (BLTString*)value.gc;
            BLTString* otherStr = (BLTString*)other->value.gc;
            return str->EqualShrStr(otherStr);
        }
    case LuaType::LNGSTR:
        {
            BLTString* str = (BLTString*)value.gc;
            BLTString* otherStr = (BLTString*)other->value.gc;
            return str->EqualLngStr(otherStr);
        }
    case LuaType::TLIGHTUSERDATA:
        {
            return value.p == other->value.p;
        }
    case LuaType::TLCF:
        {
            return value.f == other->value.f;
        }
    default:
        {
            return value.gc == other->value.gc;
        }
    }
    
    return false;
}


bool BLTValue::IsNumber()
{
    return LuaType::is_number(type);
}

bool BLTValue::IsInteger()
{
    return type == LuaType::NUMINT;
}
bool BLTValue::IsNan()
{
    return !(value.n == value.n);
}

bool BLTValue::IsShrStr()
{
    return type == LuaType::SHRSTR;
}


