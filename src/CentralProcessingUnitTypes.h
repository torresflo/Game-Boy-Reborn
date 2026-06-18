#pragma once

#include "Common.h"

struct Registers
{
    u8 A;
    u8 F;
    u8 B;
    u8 C;
    u8 D;
    u8 E;
    u8 H;
    u8 L;
    u16 SP; //Stack Pointer
    u16 PC; //Program Counter

    u16 getHL() const { return (static_cast<u16>(H) << 8) | L; }
    void setHL(u16 value)
    {
        H = static_cast<u8>(value >> 8);
        L = static_cast<u8>(value & 0xFF);
    }
};

enum InterruptType
{
    VBlank = 1,
    LCD = 2,
    Timer = 4,
    Serial = 8,
    Joypad = 16
};