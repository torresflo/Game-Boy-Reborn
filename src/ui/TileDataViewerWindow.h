#pragma once

#include <array>

#include <SFML/Graphics.hpp>

#include "Common.h"
#include "ToolWindow.h"

class MemoryBus;

class TileDataViewerWindow : public ToolWindow
{
public:
    TileDataViewerWindow();

protected:
    void drawContent(Emulator& emulator) override;

private:
    static constexpr u32 TileSize = 8;
    static constexpr u32 TileColumns = 16;
    static constexpr u32 TileRows = 24; // 16 x 24 = 384 tiles, VRAM tile data 0x8000-0x97FF
    static constexpr u32 ImageWidth = TileSize * TileColumns;
    static constexpr u32 ImageHeight = TileSize * TileRows;
    static constexpr float PixelScale = 3.f;

    static constexpr u32 WindowWidth = static_cast<u32>(ImageWidth * PixelScale);
    static constexpr u32 WindowHeight = static_cast<u32>(ImageHeight * PixelScale);

    void updateTexture(const MemoryBus& memoryBus);

    sf::Texture tileDataTexture;
    sf::Sprite tileDataSprite;
    std::array<u8, ImageWidth * ImageHeight * 4> pixels{};
};
