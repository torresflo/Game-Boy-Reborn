#pragma once

#include "Common.h"

struct ObjectAttributeMemoryEntry
{
    u8 x = 0;
    u8 y = 0;
    u8 tileIndex = 0;

    //Flags
    u8 paletteNumberCGB : 3 = 0; //Game Boy Color only
    u8 tileVRAMbankCGB : 1 = 0; //Game boy Color only
    u8 paletteNumber : 1 = 0; //Non Game boy Color only
    u8 xFlip : 1 = 0;
    u8 yFlip : 1 = 0;
    u8 backroundPriority : 1 = 0;
};
