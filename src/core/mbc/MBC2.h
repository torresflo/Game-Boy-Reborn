#pragma once

#include "Common.h"
#include "MemoryBankController.h"

// MBC2: supports up to 256KByte ROM. Has 512x4-bit RAM built into the MBC2
// chip itself (not sized by the cartridge header), occupying 0xA000-0xA1FF
// and mirrored across the rest of the 0xA000-0xBFFF window.
class MBC2 : public MemoryBankController
{
public:
    MBC2(Cartridge* cartridgePtr, u32 romBankCount, bool hasBattery);

    u8 read(u16 address) const override;
    void write(u16 address, u8 value) override;

private:
    static constexpr u32 ROMBankSizeBytes = 0x4000;
    static constexpr u32 BuiltInRAMSizeBytes = 0x200; // 512 x 4-bit nibbles

    u32 ROMBankCount;

    u8 romBankNumber = 1; // 4-bit register. Writing 0 is treated as 1.
    bool ramEnabled = false;
};
