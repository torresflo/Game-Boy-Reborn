#pragma once

#include "Common.h"
#include "MemoryBankController.h"

// Cartridge type 0x00 (ROM ONLY): a single fixed 32KB ROM, no banking, no external RAM.
class NoMemoryBankController : public MemoryBankController
{
public:
    explicit NoMemoryBankController(Cartridge* cartridgePtr);

    u8 read(u16 address) const override;
    void write(u16 address, u8 value) override;
};
