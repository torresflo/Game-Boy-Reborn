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
};

enum InterruptType
{
    VBlank = 1,
    LCD = 2,
    Timer = 4,
    Serial = 8,
    Joypad = 16
};