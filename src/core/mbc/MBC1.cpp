#include "MBC1.h"

MBC1::MBC1(Cartridge* cartridgePtr, u32 romBankCount, u32 ramBankCount, bool hasBattery)
    : MemoryBankController(cartridgePtr, hasBattery)
    , ROMBankCount(romBankCount)
    , RAMBankCount(ramBankCount)
{
}

u8 MBC1::read(u16 address) const
{
    if(address < 0x4000)
    {
        u32 bank = advancedBankingMode ? ((static_cast<u32>(secondaryBankRegister) << 5) & (ROMBankCount - 1)) : 0;
        return getROMData()[bank * ROMBankSizeBytes + address];
    }

    if(address < 0x8000)
    {
        u32 bank = ((static_cast<u32>(secondaryBankRegister) << 5) | romBankNumber) & (ROMBankCount - 1);
        return getROMData()[bank * ROMBankSizeBytes + (address - 0x4000)];
    }

    //Cartridge RAM, 0xA000-0xBFFF
    if(!ramEnabled || RAMBankCount == 0)
        return 0xFF;

    return getRAMData()[getCurrentRAMBankIndex() * RAMBankSizeBytes + (address - 0xA000)];
}

void MBC1::write(u16 address, u8 value)
{
    if(address < 0x2000)
    {
        ramEnabled = (value & 0x0F) == 0x0A;
    }
    else if(address < 0x4000)
    {
        u8 bankBits = static_cast<u8>(value & 0x1F);
        if(bankBits == 0)
            romBankNumber = 1; //Bank 0 is not selectable here, the chip substitutes bank 1
        else
            romBankNumber = bankBits;
    }
    else if(address < 0x6000)
    {
        secondaryBankRegister = static_cast<u8>(value & 0x03);
    }
    else if(address < 0x8000)
    {
        advancedBankingMode = (value & 0x01) != 0;
    }
    else if(ramEnabled && RAMBankCount > 0)
    {
        getRAMData()[getCurrentRAMBankIndex() * RAMBankSizeBytes + (address - 0xA000)] = value;
    }
}

u32 MBC1::getCurrentRAMBankIndex() const
{
    return advancedBankingMode ? (secondaryBankRegister & (RAMBankCount - 1)) : 0;
}
