#include "TileDataViewerPanel.h"

#include <algorithm>

#include <imgui.h>
#include <imgui-SFML.h>

#include "GameBoyEmulator.h"

TileDataViewerPanel::TileDataViewerPanel()
    : DebugPanel("Tile Data"), tileDataSprite(tileDataTexture)
{
    if(!tileDataTexture.resize({ImageWidth, ImageHeight}))
        Log::print(LogLevel::Error, "Failed to create the tile data texture");

    tileDataSprite.setTexture(tileDataTexture, true);
}

void TileDataViewerPanel::draw(GameBoyEmulator& emulator)
{
    if(!emulator.isROMLoaded())
        return;

    updateTexture(emulator.getPPU());

    ImGui::Image(tileDataSprite, sf::Vector2f(static_cast<float>(DisplayWidth), static_cast<float>(DisplayHeight)));
}

void TileDataViewerPanel::updateTexture(const PixelProcessingUnit& PPU)
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

                const std::array<u8, 4>& shade = tileDataUnset ? NoObjectColor : ShadeColors[colorIndex];
                pixels[pixelOffset + 0] = shade[0];
                pixels[pixelOffset + 1] = shade[1];
                pixels[pixelOffset + 2] = shade[2];
                pixels[pixelOffset + 3] = shade[3];
            }
        }
    }

    tileDataTexture.update(pixels.data());
}
