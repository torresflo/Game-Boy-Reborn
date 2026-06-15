#include "MemoryBus.h"
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
// 0xFE00 - 0xFE9F : Object Attribute Memory
// 0xFEA0 - 0xFEFF : Reserved - Unusable
// 0xFF00 - 0xFF7F : I/O Registers
// 0xFF80 - 0xFFFE : Zero Page

u8 MemoryBus::read(u16 address) const
{
    if(cartridge != nullptr)
    {
        if(address <= 0x8000)
            return cartridge->read(address);
    }

    Log::print(LogLevel::Error, "Unimplemented bus reading.");
    return u8();
}

u16 MemoryBus::read16(u16 address) const
{
    //Implicit conversion to 16 bits
    u16 lo = read(address);
    u16 hi = read(address +1);

    return (hi << 8) | lo;
}

void MemoryBus::write16(u16 address, u16 value)
{
    write(address + 1, (value >> 8) & 0xFF);
    write(address, value & 0xFF);
}

void MemoryBus::write(u16 address, u8 value)
{
    if(cartridge != nullptr)
    {
        if(address <= 0x8000)
            cartridge->write(address, value);
        return;
    }

    Log::print(LogLevel::Error, "Unimplemented bus writing.");
}

void MemoryBus::setCartridge(Cartridge* cartridgePtr)
{
    cartridge = cartridgePtr;
}
