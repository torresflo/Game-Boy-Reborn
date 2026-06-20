#include "TileDataViewerWindow.h"

#include "Emulator.h"
#include "MathUtils.h"
#include "MemoryBus.h"

namespace
{
    constexpr u16 TileDataStartAddress = 0x8000;
    constexpr u32 BytesPerTile = 16;

    // Classic DMG 4-shade palette (white, light gray, dark gray, black), RGBA8.
    constexpr std::array<std::array<u8, 4>, 4> ShadeColors =
    {{
        {0xFF, 0xFF, 0xFF, 0xFF},
        {0xAA, 0xAA, 0xAA, 0xFF},
        {0x55, 0x55, 0x55, 0xFF},
        {0x00, 0x00, 0x00, 0xFF},
    }};
}

TileDataViewerWindow::TileDataViewerWindow()
    : ToolWindow("Tile Data", WindowWidth, WindowHeight), tileDataSprite(tileDataTexture)
{
    if(!tileDataTexture.resize({ImageWidth, ImageHeight}))
        Log::print(LogLevel::Error, "Failed to create the tile data texture");

    tileDataSprite.setTexture(tileDataTexture, true);
    tileDataSprite.setScale({PixelScale, PixelScale});
}

void TileDataViewerWindow::drawContent(Emulator& emulator)
{
    if(!emulator.isROMLoaded())
        return;

    updateTexture(emulator.getMemoryBus());

    window->draw(tileDataSprite);
}

void TileDataViewerWindow::updateTexture(const MemoryBus& memoryBus)
{
    for(u32 tileIndex = 0; tileIndex < TileColumns * TileRows; ++tileIndex)
    {
        u32 tileOriginX = (tileIndex % TileColumns) * TileSize;
        u32 tileOriginY = (tileIndex / TileColumns) * TileSize;
        u16 tileAddress = TileDataStartAddress + static_cast<u16>(tileIndex * BytesPerTile);

        std::array<u8, BytesPerTile> tileBytes{};
        u8 tileByteUnion = 0;
        for(u32 byteIndex = 0; byteIndex < BytesPerTile; ++byteIndex)
        {
            tileBytes[byteIndex] = memoryBus.read(tileAddress + static_cast<u16>(byteIndex));
            tileByteUnion |= tileBytes[byteIndex];
        }
        bool tileDataUnset = (tileByteUnion == 0);

        for(u32 row = 0; row < TileSize; ++row)
        {
            u8 lowByte = tileBytes[row * 2];
            u8 highByte = tileBytes[row * 2 + 1];

            for(u32 column = 0; column < TileSize; ++column)
            {
                u32 pixelOffset = ((tileOriginY + row) * ImageWidth + (tileOriginX + column)) * 4;

                u8 colorIndex = 0;
                if(!tileDataUnset)
                {
                    u32 bit = 7 - column;
                    colorIndex = (MathUtils<u8>::getBitValue(highByte, bit) ? 2 : 0)
                               | (MathUtils<u8>::getBitValue(lowByte, bit) ? 1 : 0);
                }

                const std::array<u8, 4>& shade = tileDataUnset ? ShadeColors[3] : ShadeColors[colorIndex];
                pixels[pixelOffset + 0] = shade[0];
                pixels[pixelOffset + 1] = shade[1];
                pixels[pixelOffset + 2] = shade[2];
                pixels[pixelOffset + 3] = shade[3];
            }
        }
    }

    tileDataTexture.update(pixels.data());
}
