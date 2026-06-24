#include "MemoryBankControllerFactory.h"

#include <format>

#include "Cartridge.h"
#include "MemoryBankController.h"
#include "NoMemoryBankController.h"
#include "MBC1.h"

u32 MemoryBankControllerFactory::getROMBankCountFromHeader(u8 ROMSizeCode)
{
    return 2u << ROMSizeCode;
}

// RAM size codes are not monotonic with their byte value (0x04 is smaller than 0x05)
u32 MemoryBankControllerFactory::getRAMSizeBytesFromHeader(u8 RAMSizeCode)
{
    switch(RAMSizeCode)
    {
        case 0x02:
            return 0x2000;  // 8 KB, 1 bank
        case 0x03:
            return 0x8000;  // 32 KB, 4 banks
        case 0x04:
            return 0x20000; // 128 KB, 16 banks
        case 0x05:
            return 0x10000; // 64 KB, 8 banks
        default:
            return 0;       // 0x00: none, 0x01: unused
    }
}

std::unique_ptr<MemoryBankController> MemoryBankControllerFactory::create(Cartridge& cartridge)
{
    const CartridgeHeader& header = cartridge.getHeader();
    const u32 romBankCount = getROMBankCountFromHeader(header.romSize);

    switch(header.cartridgeType)
    {
    case 0x00: //ROM ONLY
        return std::make_unique<NoMemoryBankController>(&cartridge);

    case 0x01: //MBC1
    case 0x02: //MBC1+RAM
    case 0x03: //MBC1+RAM+BATTERY
    {
        const u32 ramSizeBytes = getRAMSizeBytesFromHeader(header.ramSize);
        cartridge.RAMData.assign(ramSizeBytes, 0);
        const bool hasBattery = header.cartridgeType == 0x03;
        return std::make_unique<MBC1>(&cartridge, romBankCount, ramSizeBytes / 0x2000, hasBattery);
    }

    default:
        Log::print(LogLevel::Error, std::format("Unsupported cartridge type: 0x{:02X}", header.cartridgeType));
        return nullptr;
    }
}
