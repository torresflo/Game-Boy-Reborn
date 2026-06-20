#include "TileDataViewerWindow.h"

#include <algorithm>

#include "Emulator.h"

namespace
{
    // Classic DMG 4-shade palette (white, light gray, dark gray, black), RGBA8.
    constexpr std::array<std::array<u8, 4>, 4> ShadeColors =
    {{
        {0xFF, 0xFF, 0xFF, 0xFF},
        {0xAA, 0xAA, 0xAA, 0xFF},
        {0x55, 0x55, 0x55, 0xFF},
        {0x00, 0x00, 0x00, 0xFF},
    }};

    // Distinct from the grayscale shades above so the grid stays visible against any tile.
    constexpr std::array<u8, 4> GridLineColor = {0x00, 0x80, 0x80, 0xFF};
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

    updateTexture(emulator.getPPU());

    window->draw(tileDataSprite);
}

void TileDataViewerWindow::updateTexture(const PixelProcessingUnit& PPU)
{
    for(u32 pixelIndex = 0; pixelIndex < ImageWidth * ImageHeight; ++pixelIndex)
    {
        std::copy(GridLineColor.begin(), GridLineColor.end(), pixels.begin() + pixelIndex * 4);
    }

    for(u32 tileIndex = 0; tileIndex < PixelProcessingUnit::TileColumns * PixelProcessingUnit::TileRows; ++tileIndex)
    {
        u32 tileOriginX = (tileIndex % PixelProcessingUnit::TileColumns) * (PixelProcessingUnit::TileSize + TileSpacing);
        u32 tileOriginY = (tileIndex / PixelProcessingUnit::TileColumns) * (PixelProcessingUnit::TileSize + TileSpacing);

        PixelProcessingUnit::Tile tile = PPU.decodeTileAIndex(tileIndex);
        bool tileDataUnset = std::all_of(tile.begin(), tile.end(), [](u8 colorIndex) { return colorIndex == 0; });

        for(u32 row = 0; row < PixelProcessingUnit::TileSize; ++row)
        {
            for(u32 column = 0; column < PixelProcessingUnit::TileSize; ++column)
            {
                u32 pixelOffset = ((tileOriginY + row) * ImageWidth + (tileOriginX + column)) * 4;
                u8 colorIndex = tile[row * PixelProcessingUnit::TileSize + column];

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
