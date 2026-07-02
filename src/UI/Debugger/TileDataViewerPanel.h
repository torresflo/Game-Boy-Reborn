#pragma once

#include <array>

#include <SFML/Graphics.hpp>

#include "Common.h"
#include "DebugPanel.h"

#include "PixelProcessingUnit.h"

class TileDataViewerPanel : public DebugPanel
{
public:
    TileDataViewerPanel();

    void draw(GameBoyEmulator& emulator) override;

private:
    static constexpr u32 TileSpacing = 1;
    static constexpr u32 ImageWidth = PixelProcessingUnit::TileSize * PixelProcessingUnit::TileColumns + TileSpacing * (PixelProcessingUnit::TileColumns - 1);
    static constexpr u32 ImageHeight = PixelProcessingUnit::TileSize * PixelProcessingUnit::TileRows + TileSpacing * (PixelProcessingUnit::TileRows - 1);
    static constexpr float PixelScale = 2.f;

    static constexpr u32 DisplayWidth = static_cast<u32>(ImageWidth * PixelScale);
    static constexpr u32 DisplayHeight = static_cast<u32>(ImageHeight * PixelScale);

    static constexpr std::array<u8, 4> GridLineColor = {0x00, 0x80, 0x80, 0xFF};
    static constexpr std::array<u8, 4> NoObjectColor = {0xFF, 0x87, 0xF6, 0xFF};
    static constexpr std::array<std::array<u8, 4>, 4> ShadeColors =
    {{
        {0xFF, 0xFF, 0xFF, 0xFF},
        {0xAA, 0xAA, 0xAA, 0xFF},
        {0x55, 0x55, 0x55, 0xFF},
        {0x00, 0x00, 0x00, 0xFF},
    }};

    void updateTexture(const PixelProcessingUnit& PPU);

    sf::Texture tileDataTexture;
    sf::Sprite tileDataSprite;
    std::array<u8, ImageWidth * ImageHeight * 4> pixels{};
};
