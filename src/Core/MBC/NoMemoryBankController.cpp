#include "NoMemoryBankController.h"

NoMemoryBankController::NoMemoryBankController(Cartridge* cartridgePtr, bool hasBatteryFlag, u32 ramSizeBytes)
    : MemoryBankController(cartridgePtr, hasBatteryFlag)
    , RAMSizeBytes(ramSizeBytes)
{
}

u8 NoMemoryBankController::read(u16 address) const
{
    if(address < 0x8000)
        return getROMData()[address];

    //Cartridge RAM, 0xA000-0xBFFF: fixed-size, no banking, no enable gating
    u32 offset = address - 0xA000;
    if(offset < RAMSizeBytes)
        return getRAMData()[offset];

    return 0xFF;
}

void NoMemoryBankController::write(u16 address, u8 value)
{
    if(address < 0x8000)
        return;

    u32 offset = address - 0xA000;
    if(offset < RAMSizeBytes)
        getRAMData()[offset] = value;
}
