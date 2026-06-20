#pragma once

#include <array>

#include <SFML/Graphics.hpp>

#include "Common.h"
#include "ToolWindow.h"

#include "PixelProcessingUnit.h"

class TileDataViewerWindow : public ToolWindow
{
public:
    TileDataViewerWindow();

protected:
    void drawContent(Emulator& emulator) override;

private:
    static constexpr u32 TileSpacing = 1;
    static constexpr u32 ImageWidth = PixelProcessingUnit::TileSize * PixelProcessingUnit::TileColumns + TileSpacing * (PixelProcessingUnit::TileColumns - 1);
    static constexpr u32 ImageHeight = PixelProcessingUnit::TileSize * PixelProcessingUnit::TileRows + TileSpacing * (PixelProcessingUnit::TileRows - 1);
    static constexpr float PixelScale = 2.f;

    static constexpr u32 WindowWidth = static_cast<u32>(ImageWidth * PixelScale);
    static constexpr u32 WindowHeight = static_cast<u32>(ImageHeight * PixelScale);

    void updateTexture(const PixelProcessingUnit& PPU);

    sf::Texture tileDataTexture;
    sf::Sprite tileDataSprite;
    std::array<u8, ImageWidth * ImageHeight * 4> pixels{};
};
