#pragma once
#include "Common.h"

class Cartdrige;

class MemoryBus
{
public:
    void setCartridge(Cartdrige* cartridge);

    u8 read(u16 address);
    void write(u16 address, u8 value);

private:
    Cartdrige* cartridge = nullptr;
};