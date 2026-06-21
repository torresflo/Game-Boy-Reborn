#include "PixelProcessingUnitTypes.h"

static std::array<u32, 4> defaultColors = {0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000};

void LCDData::initialize()
{
    control = 0x91;
    scrollY = 0;
    scrollX = 0;
    coordinateY = 0;
    compareY = 0;
    backgroundPalette = 0xFC;
    objectPaletteData.fill(0xFF);
    windowY = 0;
    windowX = 0;

    backgroundColors = defaultColors;
    object1Colors = defaultColors;
    object2Colors = defaultColors;
}

void LCDData::updatePaletteData(u8 paletteData, u8 palette)
{
    std::array<u32, 4> newPalette;
    newPalette[0] = defaultColors.at(paletteData & 0b11);
    newPalette[1] = defaultColors.at((paletteData >> 2) & 0b11);
    newPalette[2] = defaultColors.at((paletteData >> 4) & 0b11);
    newPalette[3] = defaultColors.at((paletteData >> 6) & 0b11);

    switch(palette)
    {
        case 1:
            object1Colors = newPalette;
            break;
        case 2:
            object2Colors = newPalette;
            break;
        default:
            backgroundColors = newPalette;
            break;
    }
}

void PixelFIFOContext::initialize()
{
    queue = std::queue<u32>();
    lineX = 0;
    pushedX = 0;
    fetchX = 0;
    backgroundFetchData.fill(0);
    fetchEntryData.fill(0);
    mapY = 0;
    mapX = 0;
    tileY = 0;
    fifoX = 0;
    state = PixelFIFOState::GetTile;
}
