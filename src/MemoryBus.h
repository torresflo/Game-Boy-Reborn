#pragma once
#include <array>

#include "Common.h"

class Cartridge;

class MemoryBus
{
public:
    virtual ~MemoryBus() = default;

    void setCartridge(Cartridge* cartridge);

    virtual u8 read(u16 address) const;
    u16 read16(u16 address) const;

    virtual void write(u16 address, u8 value);
    void write16(u16 address, u16 value);

private:
    u8 readWRAM(u16 address) const;
    void writeWRAM(u16 address, u8 value);

    u8 readHRAM(u16 address) const;
    void writeHRAM(u16 address, u8 value);

    u8 readInterruptEnableRegister() const;
    void writeInterruptEnableRegister(u8 value);

    u8 InterruptEnableRegister; //IE
    std::array<u8, 0x2000> WRAM;
    std::array<u8, 0x80> HRAM;

    Cartridge* cartridge = nullptr;
};