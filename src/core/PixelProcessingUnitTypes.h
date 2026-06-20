#pragma once

#include "Common.h"

struct ObjectAttributeMemoryEntry
{
    u8 x;
    u8 y;
    u8 tileIndex;

    //Flags
    u8 paletteNumberCGB : 3; //Game Boy Color only
    u8 tileVRAMbankCGB : 1; //Game boy Color only
    u8 paletteNumber : 1; //Non Game boy Color only
    u8 xFlip : 1;
    u8 yFlip : 1;
    u8 backroundPriority : 1;
};
