#include "CentralProcessingUnit.h"

#include "MemoryBus.h"
#include "MathUtils.h"

void CentralProcessingUnit::requestInterrupt(InterruptType type)
{
    u8 interruptFlags = memoryBus->readInterruptFlags();
    MathUtils<u8>::setBitValue(interruptFlags, type, true);
    memoryBus->writeInterruptFlags(interruptFlags);
}

void CentralProcessingUnit::handleInterrupts()
{
    if(handleInterrupt(InterruptType::VBlank, 0x40))
        return;
    if(handleInterrupt(InterruptType::LCD, 0x48))
        return;
    if(handleInterrupt(InterruptType::Timer, 0x50))
        return;
    if(handleInterrupt(InterruptType::Serial, 0x58))
        return;
    handleInterrupt(InterruptType::Joypad, 0x60);
}

bool CentralProcessingUnit::handleInterrupt(InterruptType type, u16 address)
{
    u8 interruptFlags = memoryBus->readInterruptFlags();

    if(MathUtils<u8>::getBitValue(interruptFlags, type)
        && MathUtils<u8>::getBitValue(memoryBus->readInterruptEnableRegister(), type))
    {
        stackPush16(registers.PC);
        registers.PC = address;
        MathUtils<u8>::setBitValue(interruptFlags, type, false);
        memoryBus->writeInterruptFlags(interruptFlags);
        halted = false;
        interruptMasterEnabled = false;
        return true;
    }

    return false;
}