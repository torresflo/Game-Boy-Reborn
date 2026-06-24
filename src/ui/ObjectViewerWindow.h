#pragma once

#include <array>

#include <SFML/Graphics.hpp>

#include "Common.h"
#include "ToolWindow.h"

#include "MemoryBus.h"
#include "PixelProcessingUnit.h"

class ObjectViewerWindow : public ToolWindow
{
public:
    ObjectViewerWindow();

protected:
    void drawContent(GameBoyEmulator& emulator) override;

private:
    static constexpr u32 ObjectsPerRow = 8;
    static constexpr u32 ObjectRows = MemoryBus::OAMEntries / ObjectsPerRow;
    static constexpr u32 ObjectCellWidth = PixelProcessingUnit::TileSize;
    static constexpr u32 ObjectCellHeight = PixelProcessingUnit::TileSize * 2; // Reserve room for 8x16 objects regardless of the current sprite size mode
    static constexpr u32 CellSpacing = 1;
    static constexpr u32 ImageWidth = ObjectCellWidth * ObjectsPerRow + CellSpacing * (ObjectsPerRow - 1);
    static constexpr u32 ImageHeight = ObjectCellHeight * ObjectRows + CellSpacing * (ObjectRows - 1);
    static constexpr float PixelScale = 4.f;
    static constexpr float TooltipCursorOffset = 8.f;

    static constexpr u32 WindowWidth = static_cast<u32>(ImageWidth * PixelScale);
    static constexpr u32 WindowHeight = static_cast<u32>(ImageHeight * PixelScale);

    void updateTexture(const PixelProcessingUnit& PPU, const MemoryBus& bus);
    void drawHoveredObjectTooltip(const MemoryBus& bus) const;
    sf::Vector2f calculateTooltipPosition(sf::Vector2i mousePosition, sf::Vector2f tooltipSize) const;

    sf::Texture objectsTexture;
    sf::Sprite objectsSprite;
    std::array<u8, ImageWidth * ImageHeight * 4> pixels{};
};
