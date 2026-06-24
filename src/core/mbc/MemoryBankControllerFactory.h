#pragma once

#include <memory>

#include "Common.h"

class MemoryBankController;
class Cartridge;

class MemoryBankControllerFactory
{
public:
    MemoryBankControllerFactory() = delete;

    static std::unique_ptr<MemoryBankController> create(Cartridge& cartridge);

private:
    static u32 getROMBankCountFromHeader(u8 ROMSizeCode);
    static u32 getRAMSizeBytesFromHeader(u8 RAMSizeCode);
};
