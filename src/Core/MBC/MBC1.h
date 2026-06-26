#pragma once

#include "Common.h"
#include "MemoryBankController.h"

// MBC1: supports up to 2MByte ROM and/or 32KByte RAM.
// The multi-cart MBC1M variant (4-bit ROM bank wrap-around) is not handled.
class MBC1 : public MemoryBankController
{
public:
    MBC1(Cartridge* cartridgePtr, u32 romBankCount, u32 ramBankCount, bool hasBattery);

    u8 read(u16 address) const override;
    void write(u16 address, u8 value) override;

    virtual void serialize(SaveStateWriter& writer) const override;
    virtual void deserialize(SaveStateReader& reader) override;

private:
    static constexpr u32 ROMBankSizeBytes = 0x4000;
    static constexpr u32 RAMBankSizeBytes = 0x2000;

    u32 getCurrentRAMBankIndex() const;

    u32 ROMBankCount;
    u32 RAMBankCount;

    u8 romBankNumber = 1; // 5-bit register, 0x2000-0x3FFF. Writing 0 is treated as 1.

    // 2-bit register, 0x4000-0x5FFF. Doubles as the RAM bank number, or the upper bits of
    // the ROM bank number for the 0x4000-0x7FFF region, depending on bankingModeSelect.
    u8 secondaryBankRegister = 0;

    bool ramEnabled = false;
    bool advancedBankingMode = false; // Banking mode select, 0x6000-0x7FFF.
};
