#include "BackgroundMapViewerPanel.h"

#include <imgui-SFML.h>

#include "GameBoyEmulator.h"
#include "PixelProcessingUnitTypes.h"

BackgroundMapViewerPanel::BackgroundMapViewerPanel()
    : DebugPanel("Background & Window Map"), mapSprite(mapTexture)
{
    if(!mapTexture.resize({MapPixels, MapPixels}))
        Log::print(LogLevel::Error, "Failed to create background map viewer texture");

    mapSprite.setTexture(mapTexture, true);
}

void BackgroundMapViewerPanel::draw(GameBoyEmulator& emulator)
{
    if(!emulator.isROMLoaded())
        return;

    const PixelProcessingUnit& ppu = emulator.getPPU();
    const MemoryBus& bus = emulator.getMemoryBus();

    drawInfoPanel(ppu);
    ImGui::Separator();

    updateTexture(ppu, bus);

    ImGui::BeginChild("##BgMapImageScroll", ImVec2(0.f, 0.f), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);

    ImGui::Image(mapSprite, sf::Vector2f(MapDisplaySize, MapDisplaySize));
    ImVec2 imageTopLeft = ImGui::GetItemRectMin();

    if(selectedMap == 0)
    {
        u8 scx = ppu.readRegister(0xFF43);
        u8 scy = ppu.readRegister(0xFF42);
        drawViewportOverlay(scx, scy, imageTopLeft);
    }

    ImGui::EndChild();
}

void BackgroundMapViewerPanel::updateTexture(const PixelProcessingUnit& ppu, const MemoryBus& bus)
{
    TileMapArea mapArea = (selectedMap == 0) ? ppu.getBackgroundTileMapArea() : ppu.getWindowTileMapArea();
    u16 mapBase = static_cast<u16>(mapArea);
    TileDataArea dataArea = ppu.getTileDataArea();

    for(u32 tileRow = 0; tileRow < MapTiles; ++tileRow)
    {
        for(u32 tileCol = 0; tileCol < MapTiles; ++tileCol)
        {
            u8 tileIndexByte = bus.read(mapBase + static_cast<u16>(tileRow * MapTiles + tileCol));

            u16 tileAddress = 0;
            if(dataArea == TileDataArea::Unsigned)
            {
                tileAddress = static_cast<u16>(0x8000u + static_cast<u32>(tileIndexByte) * PixelProcessingUnit::BytesPerTile);
            }
            else
            {
                // Signed mode: tile 0 is at 0x9000; index is interpreted as s8
                s8 signedIndex = static_cast<s8>(tileIndexByte);
                tileAddress = static_cast<u16>(0x9000 + static_cast<s32>(signedIndex) * static_cast<s32>(PixelProcessingUnit::BytesPerTile));
            }

            PixelProcessingUnit::Tile tile = ppu.decodeTileAtAddress(tileAddress);

            u32 tileOriginX = tileCol * PixelProcessingUnit::TileSize;
            u32 tileOriginY = tileRow * PixelProcessingUnit::TileSize;

            for(u32 row = 0; row < PixelProcessingUnit::TileSize; ++row)
            {
                for(u32 col = 0; col < PixelProcessingUnit::TileSize; ++col)
                {
                    u8 colorIndex = tile[row * PixelProcessingUnit::TileSize + col];
                    const auto& shade = ShadeColors[colorIndex];

                    u32 offset = ((tileOriginY + row) * MapPixels + (tileOriginX + col)) * 4;
                    pixels[offset + 0] = shade[0];
                    pixels[offset + 1] = shade[1];
                    pixels[offset + 2] = shade[2];
                    pixels[offset + 3] = shade[3];
                }
            }
        }
    }

    mapTexture.update(pixels.data());
}

void BackgroundMapViewerPanel::drawViewportOverlay(u8 scx, u8 scy, ImVec2 imageTopLeft)
{
    float mx = static_cast<float>(scx);
    float my = static_cast<float>(scy);
    float vw = static_cast<float>(ViewportWidth);
    float vh = static_cast<float>(ViewportHeight);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    auto drawOutlineRect = [&](float x, float y, float w, float h)
    {
        ImVec2 rectMin{imageTopLeft.x + x * PixelScale, imageTopLeft.y + y * PixelScale};
        ImVec2 rectMax{rectMin.x + w * PixelScale, rectMin.y + h * PixelScale};
        drawList->AddRect(rectMin, rectMax, IM_COL32(255, 80, 80, 220), 0.f, 0, 1.5f);
    };

    float wRight = (mx + vw > 256.f) ? (mx + vw - 256.f) : 0.f;
    float hBottom = (my + vh > 256.f) ? (my + vh - 256.f) : 0.f;
    float wLeft = vw - wRight;
    float hTop = vh - hBottom;

    drawOutlineRect(mx, my, wLeft, hTop);

    if(wRight > 0.f)
        drawOutlineRect(0.f, my, wRight, hTop);

    if(hBottom > 0.f)
        drawOutlineRect(mx, 0.f, wLeft, hBottom);

    if(wRight > 0.f && hBottom > 0.f)
        drawOutlineRect(0.f, 0.f, wRight, hBottom);
}

void BackgroundMapViewerPanel::drawInfoPanel(const PixelProcessingUnit& ppu)
{
    ImGui::RadioButton("Background", &selectedMap, 0);
    ImGui::SameLine(0.f, 16.f);
    ImGui::RadioButton("Window", &selectedMap, 1);
    ImGui::SameLine(0.f, 24.f);

    bool bgEnabled = ppu.getBackgroundEnabled();
    bool winEnabled = ppu.getWindowEnabled();
    bool isUnsigned = (ppu.getTileDataArea() == TileDataArea::Unsigned);

    ImGui::TextColored(NameColor, "BG:");
    ImGui::SameLine(0.f, 4.f);
    ImGui::TextColored(bgEnabled ? EnabledColor : DisabledColor, bgEnabled ? "ON" : "OFF");
    ImGui::SameLine(0.f, 12.f);

    ImGui::TextColored(NameColor, "WIN:");
    ImGui::SameLine(0.f, 4.f);
    ImGui::TextColored(winEnabled ? EnabledColor : DisabledColor, winEnabled ? "ON" : "OFF");
    ImGui::SameLine(0.f, 12.f);

    ImGui::TextColored(NameColor, "Tiles:");
    ImGui::SameLine(0.f, 4.f);
    ImGui::TextColored(ValueColor, isUnsigned ? "0x8000" : "0x8800");

    u8 scx = ppu.readRegister(0xFF43);
    u8 scy = ppu.readRegister(0xFF42);
    u8 wx = ppu.readRegister(0xFF4B);
    u8 wy = ppu.readRegister(0xFF4A);

    TileMapArea bgArea = ppu.getBackgroundTileMapArea();
    TileMapArea winArea = ppu.getWindowTileMapArea();

    ImGui::TextColored(NameColor, "SCX:");
    ImGui::SameLine(0.f, 4.f);
    ImGui::TextColored(ValueColor, "%3u", scx);
    ImGui::SameLine(0.f, 10.f);

    ImGui::TextColored(NameColor, "SCY:");
    ImGui::SameLine(0.f, 4.f);
    ImGui::TextColored(ValueColor, "%3u", scy);
    ImGui::SameLine(0.f, 10.f);

    ImGui::TextColored(NameColor, "WX:");
    ImGui::SameLine(0.f, 4.f);
    ImGui::TextColored(ValueColor, "%3u", wx);
    ImGui::SameLine(0.f, 10.f);

    ImGui::TextColored(NameColor, "WY:");
    ImGui::SameLine(0.f, 4.f);
    ImGui::TextColored(ValueColor, "%3u", wy);
    ImGui::SameLine(0.f, 16.f);

    ImGui::TextColored(NameColor, "BG map:");
    ImGui::SameLine(0.f, 4.f);
    ImGui::TextColored(ValueColor, bgArea == TileMapArea::High ? "0x9C00" : "0x9800");
    ImGui::SameLine(0.f, 10.f);

    ImGui::TextColored(NameColor, "WIN map:");
    ImGui::SameLine(0.f, 4.f);
    ImGui::TextColored(ValueColor, winArea == TileMapArea::High ? "0x9C00" : "0x9800");
}
