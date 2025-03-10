#include "blstate.h"

#include "blexception.h"
#include "blobject.h"
#include "bltype.h"

LG* BLGlobalState::lg = nullptr;
void BLGlobalState::InitStr()
{
    strt.nuse = 0;
    strt.size = 0;
    BLTString::Resize(mainThread, MINSTRTABLESIZE);
    memerrmsg = BLTString::NewString(mainThread, MEMERRMSG); 
    FixGC(memerrmsg);

    // strcache table can not hold the string objects which will be sweep soon
    for (int i = 0; i < STRCACHE_M; i ++) {
        for (int j = 0; j < STRCACHE_N; j ++)
            strcache[i][j] = memerrmsg;
    }
}

void BLGlobalState::FixGC(GCObject* o)
{
    allgc = allgc->next;
    o->next = fixgc;
    fixgc = o;
    GCHelper::reset_bits(o->marked, WHITE_BITS);
}

void BLGlobalState::CloseLuaState()
{
    delete BLGlobalState::lg;
    BLGlobalState::lg = nullptr;
}

l_mem BLGlobalState::GetDebt()
{
    l_mem debt = GCdebt;
    if(GCdebt <= 0) // // 无债务，无需 GC 工作
        {
        return 0;
        }

    debt = debt / STEPMULADJ + 1; // 将债务按 STEPMULADJ（如 100）缩小，避免后续乘法溢出。 // 确保即使债务极小（如 1 字节）也能触发至少 1 单位的回收
    debt = debt >= (MAX_LMEM / STEPMULADJ) ? MAX_LMEM : debt * GCstepmul;

    return debt; // 需要回收的内存字节数
}

void BLGlobalState::UpdateDebt(l_mem value)
{
    GCdebt = GCdebt + value;
}

void BLGlobalState::SetPause()
{
    // 计算新的内存阈值（基于当前估计值和用户配置的暂停参数）
    l_mem estimate = GCestimate / GCPAUSE;
    // 应用步进乘数并防止溢出
    estimate = (estimate * GCstepmul) >= MAX_LMEM ? MAX_LMEM : estimate * GCstepmul;

    // 计算新的债务：当前估计值 - 调整后的阈值
    l_mem debt = GCestimate - estimate;
    SetDebt(debt);
}

void BLGlobalState::SetDebt(l_mem debt)
{
    // 获取当前总内存使用量（包括已分配但未回收的内存）
    lu_mem totalbytes = GetTotalBytes();
    // 更新内存统计：扣除已处理的债务，得到实际使用的内存
    this->totalbytes = totalbytes - debt;
    // 设置新的待处理债务
    GCdebt = debt;
}

lu_mem BLGlobalState::GetTotalBytes()
{
    // 获取当前总内存使用量（包括已分配但未回收的内存）
    return totalbytes + GCdebt;
}

lu_byte BLGlobalState::GetCurrentWhite()
{
    return currentwhite & WHITE_BITS;
}

lu_byte BLGlobalState::GetOtherWhite()
{
    return currentwhite ^ WHITE_BITS;
}
