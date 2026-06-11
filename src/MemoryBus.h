#pragma once

#include "Common.h"

class MemoryBus
{
public:
    u8 read(u16 address);
    void write(u16 address, u8 value);
};