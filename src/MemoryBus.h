#pragma once
#include "Common.h"

class Cartridge;

class MemoryBus
{
public:
    void setCartridge(Cartridge* cartridge);

    u8 read(u16 address) const;
    void write(u16 address, u8 value);

private:
    Cartridge* cartridge = nullptr;
};