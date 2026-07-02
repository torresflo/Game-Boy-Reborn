#pragma once

#include <array>

#include <SFML/Graphics.hpp>
#include <imgui.h>

#include "Common.h"
#include "DebugPanel.h"

class PixelProcessingUnit;
class MemoryBus;

class BackgroundMapViewerPanel : public DebugPanel
{
public:
    BackgroundMapViewerPanel();

    void draw(GameBoyEmulator& emulator) override;

private:
    void updateTexture(const PixelProcessingUnit& ppu, const MemoryBus& bus);
    void drawViewportOverlay(u8 scx, u8 scy, ImVec2 imageTopLeft);
    void drawInfoPanel(const PixelProcessingUnit& ppu);

    static constexpr u32 MapTiles = 32;
    static constexpr u32 MapPixels = 256; // 32 * 8
    static constexpr u32 ViewportWidth = 160;
    static constexpr u32 ViewportHeight = 144;
    static constexpr float PixelScale = 2.f;
    static constexpr float MapDisplaySize = MapPixels * PixelScale;

    static constexpr ImVec4 NameColor {0.55f, 0.75f, 1.f, 1.f};
    static constexpr ImVec4 ValueColor {1.f, 0.85f, 0.35f, 1.f};
    static constexpr ImVec4 EnabledColor {0.4f, 0.9f, 0.4f, 1.f};
    static constexpr ImVec4 DisabledColor {0.5f, 0.5f, 0.5f, 1.f};

    static constexpr std::array<std::array<u8, 4>, 4> ShadeColors =
    {{
        {0xFF, 0xFF, 0xFF, 0xFF},
        {0xAA, 0xAA, 0xAA, 0xFF},
        {0x55, 0x55, 0x55, 0xFF},
        {0x00, 0x00, 0x00, 0xFF},
    }};

    std::array<u8, MapPixels * MapPixels * 4> pixels{};
    sf::Texture mapTexture;
    sf::Sprite mapSprite;

    int selectedMap = 0;  // 0 = Background, 1 = Window
};
