#include "MBC2.h"

#include "Save/SaveStateReader.h"
#include "Save/SaveStateWriter.h"

MBC2::MBC2(Cartridge* cartridgePtr, u32 romBankCount, bool hasBattery)
    : MemoryBankController(cartridgePtr, hasBattery)
    , ROMBankCount(romBankCount)
{
}

u8 MBC2::read(u16 address) const
{
    if(address < 0x4000)
        return getROMData()[address];

    if(address < 0x8000)
    {
        u32 bank = romBankNumber & (ROMBankCount - 1);
        return getROMData()[bank * ROMBankSizeBytes + (address - 0x4000)];
    }

    //Built-in RAM, 0xA000-0xA1FF, mirrored across 0xA000-0xBFFF. Only the
    //low nibble is meaningful; the upper nibble reads as set on real hardware.
    if(!ramEnabled)
        return 0xFF;

    return getRAMData()[(address - 0xA000) & (BuiltInRAMSizeBytes - 1)] | 0xF0;
}

void MBC2::write(u16 address, u8 value)
{
    if(address < 0x4000)
    {
        //RAM-enable and ROM-bank-select share this whole range, disambiguated
        //by address bit 8 rather than by a sub-range split (unlike MBC1).
        if((address & 0x0100) == 0)
        {
            ramEnabled = (value & 0x0F) == 0x0A;
        }
        else
        {
            u8 bankBits = static_cast<u8>(value & 0x0F);
            romBankNumber = (bankBits == 0) ? 1 : bankBits; //Bank 0 is not selectable, the chip substitutes bank 1
        }
        return;
    }

    if(address < 0x8000)
        return;

    if(!ramEnabled)
        return;

    getRAMData()[(address - 0xA000) & (BuiltInRAMSizeBytes - 1)] = value & 0x0F;
}

void MBC2::serialize(SaveStateWriter& writer) const
{
    writer.write(romBankNumber);
    writer.write(ramEnabled);
}

void MBC2::deserialize(SaveStateReader& reader)
{
    reader.read(romBankNumber);
    reader.read(ramEnabled);
}
