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

    u16 getAF() const;
    u16 getBC() const;
    u16 getDE() const;
    u16 getHL() const;
    void setHL(u16 value);
};

// Bit positions within the IF/IE registers, for use with getBitValue/setBitValue.
enum InterruptType
{
    VBlank = 0,
    LCD = 1,
    Timer = 2,
    Serial = 3,
    Joypad = 4
};