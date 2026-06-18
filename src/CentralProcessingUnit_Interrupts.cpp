#include "CentralProcessingUnit.h"

#include <MathUtils.h>

u8 CentralProcessingUnit::getInterruptFlags() const
{
    return interruptFlags;
}

void CentralProcessingUnit::setInterruptFlags(u8 value)
{
    interruptFlags = value;
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
    if(MathUtils<u8>::getBitValue(interruptFlags, type)
        && MathUtils<u8>::getBitValue(interruptEnable, type))
    {
        stackPush16(readRegister(RegisterType::PC));
        writeRegister(RegisterType::PC, address);
        interruptFlags &= ~type;
        halted = false;
        interruptMasterEnabled = false;
        return true;
    }

    return false;
}

void CentralProcessingUnit::requestInterrupt([[maybe_unused]] InterruptType type)
{

}
