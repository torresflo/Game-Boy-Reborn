#pragma once

#include "Common.h"
#include "MemoryBankController.h"

// Cartridge type 0x00 (ROM ONLY): a single fixed 32KB ROM, no banking, no external RAM.
// Also covers types 0x08/0x09 (ROM+RAM[+BATTERY]): same fixed ROM, plus a fixed-size
// external RAM chip wired straight to 0xA000-0xBFFF with no RAM-enable gating.
class NoMemoryBankController : public MemoryBankController
{
public:
    NoMemoryBankController(Cartridge* cartridgePtr, bool hasBatteryFlag, u32 ramSizeBytes);

    u8 read(u16 address) const override;
    void write(u16 address, u8 value) override;

private:
    u32 RAMSizeBytes;
};
