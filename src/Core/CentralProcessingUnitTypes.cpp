#include "CentralProcessingUnitTypes.h"

u16 Registers::getAF() const
{ 
    return (static_cast<u16>(A) << 8) | F;
}

u16 Registers::getBC() const
{ 
    return (static_cast<u16>(B) << 8) | C;
}

u16 Registers::getDE() const
{ 
    return (static_cast<u16>(D) << 8) | E;
}

u16 Registers::getHL() const
{ 
    return (static_cast<u16>(H) << 8) | L;
}

void Registers::setHL(u16 value)
{
    H = static_cast<u8>(value >> 8);
    L = static_cast<u8>(value & 0xFF);
}
