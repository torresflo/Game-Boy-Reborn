#pragma once

#include "Common.h"
#include "MemoryBankController.h"

// MBC5: supports up to 8MByte ROM and/or 128KByte RAM. Unlike MBC1/MBC3,
// ROM bank 0 is a validly selectable bank (no "substitute 0 with 1" rule).
// Rumble variants repurpose RAM-bank-register bit 3 as a motor control bit,
// which this emulator has no haptic output for and simply discards.
class MBC5 : public MemoryBankController
{
public:
    MBC5(Cartridge* cartridgePtr, u32 romBankCount, u32 ramBankCount, bool hasBattery, bool hasRumble);

    u8 read(u16 address) const override;
    void write(u16 address, u8 value) override;

private:
    static constexpr u32 ROMBankSizeBytes = 0x4000;
    static constexpr u32 RAMBankSizeBytes = 0x2000;

    u32 getCurrentROMBankIndex() const;
    u32 getCurrentRAMBankIndex() const;

    u32 ROMBankCount;
    u32 RAMBankCount;
    bool rumblePresent;

    u8 romBankNumberLow = 0;           // 8-bit register, 0x2000-0x2FFF
    bool romBankNumberHighBit = false; // Bit 8, 0x3000-0x3FFF (only bit 0 of the written value matters)
    u8 ramBankNumber = 0;              // 4-bit register, 0x4000-0x5FFF (bit 3 is the rumble-motor bit on rumble carts)
    bool ramEnabled = false;
};
