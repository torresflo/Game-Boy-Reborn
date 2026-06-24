#include "NoMemoryBankController.h"

NoMemoryBankController::NoMemoryBankController(Cartridge* cartridgePtr)
    : MemoryBankController(cartridgePtr, false)
{
}

u8 NoMemoryBankController::read(u16 address) const
{
    if(address < 0x8000)
        return getROMData()[address];

    //No external RAM on a ROM ONLY cartridge
    return 0xFF;
}

void NoMemoryBankController::write(u16 address, u8 value)
{
    UNUSED(address);
    UNUSED(value);
}
