#pragma once
#include "Common.h"

class Cartridge;

class MemoryBus
{
public:
    void setCartridge(Cartridge* cartridge);

    u8 read(u16 address) const;
    u16 read16(u16 address) const;
    
    void write16(u16 address, u16 value);
    void write(u16 address, u8 value);

private:
    Cartridge* cartridge = nullptr;
};