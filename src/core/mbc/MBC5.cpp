#include "MBC5.h"

#include "save/SaveStateReader.h"
#include "save/SaveStateWriter.h"

MBC5::MBC5(Cartridge* cartridgePtr, u32 romBankCount, u32 ramBankCount, bool hasBattery, bool hasRumble)
    : MemoryBankController(cartridgePtr, hasBattery)
    , ROMBankCount(romBankCount)
    , RAMBankCount(ramBankCount)
    , rumblePresent(hasRumble)
{
}

u8 MBC5::read(u16 address) const
{
    if(address < 0x4000)
        return getROMData()[address];

    if(address < 0x8000)
        return getROMData()[getCurrentROMBankIndex() * ROMBankSizeBytes + (address - 0x4000)];

    //Cartridge RAM, 0xA000-0xBFFF
    if(!ramEnabled || RAMBankCount == 0)
        return 0xFF;

    return getRAMData()[getCurrentRAMBankIndex() * RAMBankSizeBytes + (address - 0xA000)];
}

void MBC5::write(u16 address, u8 value)
{
    if(address < 0x2000)
    {
        ramEnabled = (value & 0x0F) == 0x0A;
    }
    else if(address < 0x3000)
    {
        romBankNumberLow = value;
    }
    else if(address < 0x4000)
    {
        romBankNumberHighBit = (value & 0x01) != 0;
    }
    else if(address < 0x6000)
    {
        ramBankNumber = static_cast<u8>(value & 0x0F);
    }
    else if(address >= 0xA000 && ramEnabled && RAMBankCount > 0)
    {
        getRAMData()[getCurrentRAMBankIndex() * RAMBankSizeBytes + (address - 0xA000)] = value;
    }
}

u32 MBC5::getCurrentROMBankIndex() const
{
    u32 bank = (static_cast<u32>(romBankNumberHighBit) << 8) | romBankNumberLow;
    return bank & (ROMBankCount - 1);
}

u32 MBC5::getCurrentRAMBankIndex() const
{
    u8 bankBits = rumblePresent ? static_cast<u8>(ramBankNumber & 0x07) : ramBankNumber;
    return bankBits & (RAMBankCount - 1);
}

void MBC5::serialize(SaveStateWriter& writer) const
{
    writer.write(romBankNumberLow);
    writer.write(romBankNumberHighBit);
    writer.write(ramBankNumber);
    writer.write(ramEnabled);
}

void MBC5::deserialize(SaveStateReader& reader)
{
    reader.read(romBankNumberLow);
    reader.read(romBankNumberHighBit);
    reader.read(ramBankNumber);
    reader.read(ramEnabled);
}
