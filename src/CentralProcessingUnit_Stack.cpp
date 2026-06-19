#include "CentralProcessingUnit.h"

#include "MemoryBus.h"

void CentralProcessingUnit::stackPush(u8 data)
{
    registers.SP--;
    memoryBus->write(registers.SP, data);
}

void CentralProcessingUnit::stackPush16(u16 data)
{
    stackPush((data >> 8) & 0xFF);
    stackPush(data & 0xFF);
}

u8 CentralProcessingUnit::stackPop()
{
    u8 data = memoryBus->read(registers.SP);
    registers.SP++;
    return data;
}

u16 CentralProcessingUnit::stackPop16()
{
    //Implicit conversions
    u16 lo = stackPop();
    u16 hi = stackPop();

    return (hi << 8) | lo;
}
