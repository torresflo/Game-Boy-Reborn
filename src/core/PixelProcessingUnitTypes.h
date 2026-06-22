#pragma once

#include <array>
#include <queue>

#include "Common.h"

struct ObjectAttributeMemoryEntry
{
    u8 y = 0;
    u8 x = 0;
    u8 tileIndex = 0;

    //Flags
    u8 paletteNumberCGB : 3 = 0; //Game Boy Color only
    u8 tileVRAMbankCGB : 1 = 0; //Game boy Color only
    u8 paletteNumber : 1 = 0; //Non Game boy Color only
    u8 xFlip : 1 = 0;
    u8 yFlip : 1 = 0;
    u8 backgroundPriority : 1 = 0;
};

struct LCDData
{
    //Registers
    u8 control;                         // LCDC - 0xFF40
    u8 status;                          // STAT - 0xFF41
    u8 scrollY;                         // SCY - 0xFF42
    u8 scrollX;                         // SCX - 0xFF43
    u8 coordinateY;                     // LY - 0xFF44
    u8 compareY;                        // LYC - 0xFF45
    u8 backgroundPalette;               // BGP - 0xFF47
    std::array<u8, 2> objectPaletteData;// OBP0 - 0xFF48 | OBP1 - 0xFF49
    u8 windowY;                         // WY - 0xFF4A
    u8 windowX;                         // WX - 0xFF4B

    //Other data
    std::array<u32, 4> backgroundColors;
    std::array<u32, 4> object1Colors;
    std::array<u32, 4> object2Colors;

    void initialize();
    void updatePaletteData(u8 paletteData, u8 palette);
};

enum class LCDMode : u8
{
    HorizontalBlank = 0,
    VerticalBlank = 1,
    ObjectAccessMemoryScan = 2,
    PixelDrawing = 3
};

enum class TileMapArea : u16
{
    Low = 0x9800,
    High = 0x9C00
};

enum class TileDataArea : u16
{
    Signed = 0x8800,  //Tile index is signed (-128 to 127), tile 0 at 0x9000
    Unsigned = 0x8000 //Tile index is unsigned (0 to 255), tile 0 at 0x8000
};

enum class SpriteSize : u32
{
    EightByEight = 8,
    EightBySixteen = 16
};

enum class PixelFIFOState
{
    GetTile,
    GetTileDataLow, //backgroundFetchData[1]
    GetTileDataHigh,//backgroundFetchData[2]
    Sleep,
    Push
};

struct PixelFIFOContext
{
    PixelFIFOState state = PixelFIFOState::GetTile;
    std::queue<u32> queue; //u32 = color
    u8 lineX = 0;
    u8 pushedX = 0;
    u8 fetchX = 0;
    std::array<u8, 3> backgroundFetchData;
    std::array<u8, 6> fetchEntryData; //Low/high tile bitplane bytes for up to 3 fetched objects
    u8 mapY = 0;
    u8 mapX = 0;
    u8 tileY = 0;
    u8 fifoX = 0;

    void initialize();
};