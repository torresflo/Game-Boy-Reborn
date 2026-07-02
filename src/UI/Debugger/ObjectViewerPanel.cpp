#include "ObjectViewerPanel.h"

#include <algorithm>
#include <format>

#include <imgui.h>
#include <imgui-SFML.h>

#include "GameBoyEmulator.h"
#include "MathUtils.h"

std::array<u8, 4> ObjectViewerPanel::decodeShade(u32 color)
{
    u8 gray = static_cast<u8>((color >> 16) & 0xFF);
    u8 alpha = static_cast<u8>((color >> 24) & 0xFF);
    return {gray, gray, gray, alpha};
}

ObjectViewerPanel::ObjectViewerPanel()
    : DebugPanel("Objects (sprites)"), objectsSprite(objectsTexture)
{
    if(!objectsTexture.resize({ImageWidth, ImageHeight}))
        Log::print(LogLevel::Error, "Failed to create the objects texture");

    objectsSprite.setTexture(objectsTexture, true);
}

void ObjectViewerPanel::draw(GameBoyEmulator& emulator)
{
    if(!emulator.isROMLoaded())
        return;

    updateTexture(emulator.getPPU(), emulator.getMemoryBus());

    ImGui::Image(objectsSprite, sf::Vector2f(static_cast<float>(DisplayWidth), static_cast<float>(DisplayHeight)));
    ImVec2 imageTopLeft = ImGui::GetItemRectMin();

    drawHoveredObjectTooltip(emulator.getMemoryBus(), imageTopLeft);
}

void ObjectViewerPanel::updateTexture(const PixelProcessingUnit& PPU, const MemoryBus& bus)
{
    for(u32 pixelIndex = 0; pixelIndex < ImageWidth * ImageHeight; ++pixelIndex)
    {
        std::copy(GridLineColor.begin(), GridLineColor.end(), pixels.begin() + pixelIndex * 4);
    }

    bool isEightBySixteen = PPU.getObjectSize() == SpriteSize::EightBySixteen;
    u32 objectHeight = isEightBySixteen ? PixelProcessingUnit::TileSize * 2 : PixelProcessingUnit::TileSize;

    for(u32 objectIndex = 0; objectIndex < MemoryBus::OAMEntries; ++objectIndex)
    {
        ObjectAttributeMemoryEntry object = bus.readObject(static_cast<u16>(objectIndex));

        u32 cellOriginX = (objectIndex % ObjectsPerRow) * (ObjectCellWidth + CellSpacing);
        u32 cellOriginY = (objectIndex / ObjectsPerRow) * (ObjectCellHeight + CellSpacing);

        u8 topTileIndex = object.tileIndex;
        if(isEightBySixteen)
            MathUtils<u8>::setBitValue(topTileIndex, 0, false); //Lower bit of tile index is ignored for 8x16 objects

        PixelProcessingUnit::Tile topTile = PPU.decodeTileAIndex(topTileIndex);
        PixelProcessingUnit::Tile bottomTile{};
        if(isEightBySixteen)
            bottomTile = PPU.decodeTileAIndex(topTileIndex | 0x01);

        //Y-flipping an 8x16 object swaps its two tiles in addition to flipping each one vertically
        if(object.yFlip)
            std::swap(topTile, bottomTile);

        bool isBlank = std::all_of(topTile.begin(), topTile.end(), [](u8 colorIndex) { return colorIndex == 0; })
            && (!isEightBySixteen || std::all_of(bottomTile.begin(), bottomTile.end(), [](u8 colorIndex) { return colorIndex == 0; }));

        const std::array<u32, 4>& colors = PPU.getObjectColors(object.paletteNumber);

        for(u32 row = 0; row < objectHeight; ++row)
        {
            const PixelProcessingUnit::Tile& tile = (row < PixelProcessingUnit::TileSize) ? topTile : bottomTile;

            u32 rowInTile = row % PixelProcessingUnit::TileSize;
            if(object.yFlip)
                rowInTile = PixelProcessingUnit::TileSize - 1 - rowInTile;

            for(u32 column = 0; column < PixelProcessingUnit::TileSize; ++column)
            {
                u32 columnInTile = object.xFlip ? (PixelProcessingUnit::TileSize - 1 - column) : column;
                u8 colorIndex = tile[rowInTile * PixelProcessingUnit::TileSize + columnInTile];

                u32 pixelOffset = ((cellOriginY + row) * ImageWidth + (cellOriginX + column)) * 4;
                const std::array<u8, 4> shade = isBlank ? NoObjectColor : decodeShade(colors[colorIndex]);
                std::copy(shade.begin(), shade.end(), pixels.begin() + pixelOffset);
            }
        }
    }

    objectsTexture.update(pixels.data());
}

void ObjectViewerPanel::drawHoveredObjectTooltip(const MemoryBus& bus, ImVec2 imageTopLeft) const
{
    if(!ImGui::IsItemHovered())
        return;

    ImVec2 mousePosition = ImGui::GetMousePos();
    float imageX = (mousePosition.x - imageTopLeft.x) / PixelScale;
    float imageY = (mousePosition.y - imageTopLeft.y) / PixelScale;

    if(imageX < 0.f || imageY < 0.f || imageX >= ImageWidth || imageY >= ImageHeight)
        return;

    u32 column = static_cast<u32>(imageX) / (ObjectCellWidth + CellSpacing);
    u32 row = static_cast<u32>(imageY) / (ObjectCellHeight + CellSpacing);
    if(column >= ObjectsPerRow || row >= ObjectRows)
        return;

    u32 objectIndex = row * ObjectsPerRow + column;
    ObjectAttributeMemoryEntry object = bus.readObject(static_cast<u16>(objectIndex));

    std::string text = std::format("Object #{}\nX: {}  Y: {}\nTile: 0x{:02X}\nPalette: {}\nX Flip: {}  Y Flip: {}\nBG Priority: {}",
        objectIndex, object.x, object.y, object.tileIndex, static_cast<u32>(object.paletteNumber),
        object.xFlip ? "Yes" : "No", object.yFlip ? "Yes" : "No", object.backgroundPriority ? "Yes" : "No");

    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
    ImVec2 padding = ImGui::GetStyle().WindowPadding;
    ImVec2 tooltipSize{textSize.x + padding.x * 2.f, textSize.y + padding.y * 2.f};

    ImVec2 tooltipPosition = calculateTooltipPosition(mousePosition, tooltipSize);

    ImGui::SetNextWindowPos(tooltipPosition, ImGuiCond_Always);
    ImGui::BeginTooltip();
    ImGui::TextUnformatted(text.c_str());
    ImGui::EndTooltip();
}

ImVec2 ObjectViewerPanel::calculateTooltipPosition(ImVec2 mousePosition, ImVec2 tooltipSize) const
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workMin = viewport->WorkPos;
    ImVec2 workMax{viewport->WorkPos.x + viewport->WorkSize.x, viewport->WorkPos.y + viewport->WorkSize.y};

    float tooltipX = mousePosition.x + TooltipCursorOffset;
    if(tooltipX + tooltipSize.x > workMax.x)
        tooltipX = mousePosition.x - TooltipCursorOffset - tooltipSize.x;

    float tooltipY = mousePosition.y + TooltipCursorOffset;
    if(tooltipY + tooltipSize.y > workMax.y)
        tooltipY = mousePosition.y - TooltipCursorOffset - tooltipSize.y;

    tooltipX = std::max(workMin.x, std::min(tooltipX, workMax.x - tooltipSize.x));
    tooltipY = std::max(workMin.y, std::min(tooltipY, workMax.y - tooltipSize.y));

    return {tooltipX, tooltipY};
}
