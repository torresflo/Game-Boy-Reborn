#pragma once

#include <array>

#include <SFML/Graphics.hpp>
#include <imgui.h>

#include "Common.h"
#include "DebugPanel.h"

#include "MemoryBus.h"
#include "PixelProcessingUnit.h"

class ObjectViewerPanel : public DebugPanel
{
public:
    ObjectViewerPanel();

    void draw(GameBoyEmulator& emulator) override;

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

    static constexpr u32 DisplayWidth = static_cast<u32>(ImageWidth * PixelScale);
    static constexpr u32 DisplayHeight = static_cast<u32>(ImageHeight * PixelScale);

    static constexpr std::array<u8, 4> GridLineColor = {0x00, 0x80, 0x80, 0xFF};
    static constexpr std::array<u8, 4> NoObjectColor = {0xFF, 0x87, 0xF6, 0xFF};

    static std::array<u8, 4> decodeShade(u32 color);

    void updateTexture(const PixelProcessingUnit& PPU, const MemoryBus& bus);
    void drawHoveredObjectTooltip(const MemoryBus& bus, ImVec2 imageTopLeft) const;
    ImVec2 calculateTooltipPosition(ImVec2 mousePosition, ImVec2 tooltipSize) const;

    sf::Texture objectsTexture;
    sf::Sprite objectsSprite;
    std::array<u8, ImageWidth * ImageHeight * 4> pixels{};
};
