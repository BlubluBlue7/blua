#pragma once

#include "blluastate.h"
#include "blstring.h"

class BLGlobalState;
class BLTValue;
class LG;

enum class CALL_STATUS
{
    CALL_OK = 0,
    CALL_ERRERR = 1,
    CALL_ERRMEM = 2,
    CALL_ERRRUN = 3
};

constexpr int MIN_STACK = 20;

class CallS
{
public:
    int funcIndex;
    int nResults;
};

class BLGlobalState
{
private:
    
public:
    BLGlobalState() = default;
    ~BLGlobalState() = default;
    
    static void CloseLuaState();
    static LG* lg;
    
    BLLuaState* mainThread;
    void* ud;
    lua_CFunction panic;

    // string
    static constexpr int STRCACHE_M = 53;
    static constexpr int STRCACHE_N = 2;
    static constexpr int MINSTRTABLESIZE = 128;
    static constexpr const char* MEMERRMSG = "not enough memory";
    
    BLTString* strcache[STRCACHE_M][STRCACHE_N] = {nullptr};
    StringTable strt;
    unsigned int seed;
    BLTString* memerrmsg;
    
    void InitStr();

    // gc
    constexpr static int STEPMULADJ = 200; // 用于调整 GCstepmul 的值，使其更合理
    constexpr static int GCPAUSE = 100; // 暂停参数，用于调整 GC 的暂停阈值 200的含义是指：一轮GC结束后，Lua虚拟机的内存大小达到上一轮GC结束时刻Lua虚拟机实际内存的两倍时，才开始下一轮GC
    constexpr static int GCSTEPMUL = 200;  //用于调节GC步骤触发间隔以及单次GC步骤处理GCObject的数量用的
    constexpr static int GCSTEPSIZE = 1024; 
    constexpr static l_mem MAX_LMEM = std::numeric_limits<l_mem>::max(); // 最大内存值
    
    State gcstate;
    lu_byte currentwhite;
    GCObject* fixgc;
    GCObject* allgc;         // gc root set
    GCObject** sweepgc;
    GCObject* gray;
    GCObject* grayagain;
    lu_mem totalbytes;              // 虚拟机预设内存总大小. 真实的内存大小是totalbytes+GCdebt
    l_mem GCdebt;                   // GCdebt will be negative 当前未回收的内存债务（字节数），正值表示需要回收的内存，负值表示已超额回收
    lu_mem GCmemtrav;               // per gc step traverse memory bytes 
    lu_mem GCestimate;              // after finish a gc cycle,it records total memory bytes (totalbytes + GCdebt) 一轮GC结束之后，Lua虚拟机的实际内存大小会被赋值到这个变量中
    int GCstepmul;                  // GC 步进乘数（默认 100），控制 GC 的激进程度 值越大，单步回收的内存越多，GC 更高效但可能引入卡顿；值越小，GC 更渐进但整体耗时更长
    
    void FixGC(GCObject* o);
    l_mem GetDebt();
    lu_mem GetTotalBytes();
    void UpdateDebt(l_mem value);
    void SetPause();
    void SetDebt(l_mem debt);
    lu_byte GetCurrentWhite();
    lu_byte GetOtherWhite();
};

class LX
{
public:
    BLLuaState l;
};

class LG
{
public:
    LX lx;
    BLGlobalState g;
};

