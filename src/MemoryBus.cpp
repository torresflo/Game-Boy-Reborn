#include "MemoryBus.h"

#include <format>

#include "Cartridge.h"

// 0x0000 - 0x3FFF : ROM Bank 0
// 0x4000 - 0x7FFF : ROM Bank 1 - Switchable
// 0x8000 - 0x97FF : CHR RAM
// 0x9800 - 0x9BFF : BG Map 1
// 0x9C00 - 0x9FFF : BG Map 2
// 0xA000 - 0xBFFF : Cartridge RAM
// 0xC000 - 0xCFFF : RAM Bank 0
// 0xD000 - 0xDFFF : RAM Bank 1-7 - switchable - Color only
// 0xE000 - 0xFDFF : Reserved - Echo RAM
// 0xFE00 - 0xFE9F : OAM (Object Attribute Memory)
// 0xFEA0 - 0xFEFF : Reserved - Unusable
// 0xFF00 - 0xFF7F : I/O Registers
// 0xFF80 - 0xFFFE : Zero Page / HRAM

u8 MemoryBus::read(u16 address) const
{
    if(address < 0x8000)
    {
        //ROM Banks
        if(cartridge != nullptr)
            return cartridge->read(address);

        Log::print(LogLevel::Error, std::format("Reading ROM (0x{:4X}) with no cartridge attached.", address));
        return 0xFF;
    }
    else if(address < 0xA000)
    {
        //Char/Map Data
        Log::print(LogLevel::Error, std::format("Unsupported bus reading (0x{:4X}).", address));
    }
    else if(address < 0xC000)
    {
        //Cartridge RAM
        if(cartridge != nullptr)
            return cartridge->read(address);

        Log::print(LogLevel::Error, std::format("Reading cartridge RAM (0x{:4X}) with no cartridge attached.", address));
        return 0xFF;
    }
    else if(address < 0xE000)
    {
        //RAM Banks - WRAM (working RAM)
        return readWRAM(address);
    }
    else if(address < 0xFE00)
    {
        //Reserved Echo RAM
        Log::print(LogLevel::Error, std::format("Unsupported bus reading (0x{:4X}).", address));
    }
    else if(address < 0xFEA0)
    {
        //OAM
        Log::print(LogLevel::Error, std::format("Unsupported bus reading (0x{:4X}).", address));
    }
    else if(address < 0xFF00)
    {
        //Reserved, unusable
        Log::print(LogLevel::Error, std::format("Unsupported bus reading (0x{:4X}).", address));
    }
    else if(address < 0xFF80)
    {
        //IO Registers
        Log::print(LogLevel::Error, std::format("Unsupported bus reading (0x{:4X}).", address));
    }
    else if(address == 0xFFFF)
    {
        return readInterruptEnableRegister();
    }
    else
    {
        return readHRAM(address);
    }

    return 0xFF;
}

u16 MemoryBus::read16(u16 address) const
{
    //Implicit conversion to 16 bits
    u16 lo = read(address);
    u16 hi = read(address +1);

    return (hi << 8) | lo;
}

void MemoryBus::write(u16 address, u8 value)
{
    if(address <= 0x8000)
    {
        if(cartridge != nullptr)
            cartridge->write(address, value);
        else
            Log::print(LogLevel::Error, std::format("Writing ROM (0x{:4X}) with no cartridge attached.", address));
    }
    else if(address < 0xA000)
    {
        //Char/Map Data
        Log::print(LogLevel::Error, std::format("Unsupported bus writing (0x{:4X}).", address));
    }
    else if(address < 0xC000)
    {
        //Cartridge RAM
        if(cartridge != nullptr)
            cartridge->write(address, value);
        else
            Log::print(LogLevel::Error, std::format("Writing cartridge RAM (0x{:4X}) with no cartridge attached.", address));
    }
    else if(address < 0xE000)
    {
        //RAM Banks - WRAM (working RAM)
        writeWRAM(address, value);
    }
    else if(address < 0xFE00)
    {
        //Reserved Echo RAM
        Log::print(LogLevel::Error, std::format("Unsupported bus writing (0x{:4X}).", address));
    }
    else if(address < 0xFEA0)
    {
        //OAM
        Log::print(LogLevel::Error, std::format("Unsupported bus writing (0x{:4X}).", address));
    }
    else if(address < 0xFF00)
    {
        //Reserved, unusable
        Log::print(LogLevel::Error, std::format("Unsupported bus writing (0x{:4X}).", address));
    }
    else if(address < 0xFF80)
    {
        //IO Registers
        Log::print(LogLevel::Error, std::format("Unsupported bus writing (0x{:4X}).", address));
    }
    else if(address == 0xFFFF)
    {
        //CPU Enable Register
        writeInterruptEnableRegister(value);
    }
    else
    {
        writeHRAM(address, value);
    }
}

void MemoryBus::write16(u16 address, u16 value)
{
    write(address + 1, (value >> 8) & 0xFF);
    write(address, value & 0xFF);
}

u8 MemoryBus::readWRAM(u16 address) const
{
    u16 offset = address - 0xC000;
    if(offset >= WRAM.size())
    {
        Log::print(LogLevel::Fatal, std::format("WRAM read out of range (0x{:4X}).", address));
        exit(-1);
    }

    return WRAM[offset];
}

void MemoryBus::writeWRAM(u16 address, u8 value)
{
    u16 offset = address - 0xC000;
    if(offset >= WRAM.size())
    {
        Log::print(LogLevel::Fatal, std::format("WRAM write out of range (0x{:4X}).", address));
        exit(-1);
    }

    WRAM[offset] = value;
}

u8 MemoryBus::readHRAM(u16 address) const
{
    u16 offset = address - 0xFF80;
    if(offset >= HRAM.size())
    {
        Log::print(LogLevel::Fatal, std::format("HRAM read out of range (0x{:4X}).", address));
        exit(-1);
    }

    return HRAM[offset];
}

void MemoryBus::writeHRAM(u16 address, u8 value)
{
    u16 offset = address - 0xFF80;
    if(offset >= HRAM.size())
    {
        Log::print(LogLevel::Fatal, std::format("HRAM write out of range (0x{:4X}).", address));
        exit(-1);
    }

    HRAM[offset] = value;
}

u8 MemoryBus::readInterruptEnableRegister() const
{
    return InterruptEnableRegister;
}

void MemoryBus::writeInterruptEnableRegister(u8 value)
{
    InterruptEnableRegister = value;
}

void MemoryBus::setCartridge(Cartridge* cartridgePtr)
{
    cartridge = cartridgePtr;
}
