#include "MemoryBus.h"

u8 MemoryBus::read(u16 address)
{
    (void)(address);

    return u8();
}

void MemoryBus::write(u16 address, u8 value)
{
    (void)(address);
    (void)(value);
}
