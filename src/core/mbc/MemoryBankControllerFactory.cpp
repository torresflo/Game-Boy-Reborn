#include "MemoryBankControllerFactory.h"

#include <format>

#include "Cartridge.h"
#include "MemoryBankController.h"
#include "NoMemoryBankController.h"
#include "MBC1.h"
#include "MBC2.h"
#include "MBC3.h"
#include "MBC5.h"

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
        return std::make_unique<NoMemoryBankController>(&cartridge, false, 0);

    case 0x01: //MBC1
    case 0x02: //MBC1+RAM
    case 0x03: //MBC1+RAM+BATTERY
    {
        const u32 ramSizeBytes = getRAMSizeBytesFromHeader(header.ramSize);
        cartridge.RAMData.assign(ramSizeBytes, 0);
        const bool hasBattery = header.cartridgeType == 0x03;
        return std::make_unique<MBC1>(&cartridge, romBankCount, ramSizeBytes / 0x2000, hasBattery);
    }

    case 0x05: //MBC2
    case 0x06: //MBC2+BATTERY
    {
        //MBC2's 512x4-bit RAM is built into the chip; header.ramSize is ignored.
        cartridge.RAMData.assign(0x200, 0);
        const bool hasBattery = header.cartridgeType == 0x06;
        return std::make_unique<MBC2>(&cartridge, romBankCount, hasBattery);
    }

    case 0x08: //ROM+RAM
    case 0x09: //ROM+RAM+BATTERY
    {
        const u32 ramSizeBytes = getRAMSizeBytesFromHeader(header.ramSize);
        cartridge.RAMData.assign(ramSizeBytes, 0);
        const bool hasBattery = header.cartridgeType == 0x09;
        return std::make_unique<NoMemoryBankController>(&cartridge, hasBattery, ramSizeBytes);
    }

    case 0x0F: //MBC3+TIMER+BATTERY
    case 0x10: //MBC3+TIMER+RAM+BATTERY
    case 0x11: //MBC3
    case 0x12: //MBC3+RAM
    case 0x13: //MBC3+RAM+BATTERY
    {
        const u32 ramSizeBytes = getRAMSizeBytesFromHeader(header.ramSize);
        cartridge.RAMData.assign(ramSizeBytes, 0);
        const bool hasBattery = header.cartridgeType == 0x0F || header.cartridgeType == 0x10 || header.cartridgeType == 0x13;
        const bool hasTimer = header.cartridgeType == 0x0F || header.cartridgeType == 0x10;
        return std::make_unique<MBC3>(&cartridge, romBankCount, ramSizeBytes / 0x2000, hasBattery, hasTimer);
    }

    case 0x19: //MBC5
    case 0x1A: //MBC5+RAM
    case 0x1B: //MBC5+RAM+BATTERY
    case 0x1C: //MBC5+RUMBLE
    case 0x1D: //MBC5+RUMBLE+RAM
    case 0x1E: //MBC5+RUMBLE+RAM+BATTERY
    {
        const u32 ramSizeBytes = getRAMSizeBytesFromHeader(header.ramSize);
        cartridge.RAMData.assign(ramSizeBytes, 0);
        const bool hasBattery = header.cartridgeType == 0x1B || header.cartridgeType == 0x1E;
        const bool hasRumble = header.cartridgeType == 0x1C || header.cartridgeType == 0x1D || header.cartridgeType == 0x1E;
        return std::make_unique<MBC5>(&cartridge, romBankCount, ramSizeBytes / 0x2000, hasBattery, hasRumble);
    }

    default:
        Log::print(LogLevel::Error, std::format("Unsupported cartridge type: 0x{:02X}", header.cartridgeType));
        return nullptr;
    }
}
