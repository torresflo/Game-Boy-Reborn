#pragma once

#include <array>

#include <SFML/Graphics.hpp>
#include <imgui.h>

#include "Common.h"
#include "ToolWindow.h"

class GameBoyEmulator;
class PixelProcessingUnit;
class MemoryBus;

class BackgroundMapViewerWindow : public ToolWindow
{
public:
    BackgroundMapViewerWindow();

protected:
    void drawContent(GameBoyEmulator& emulator) override;

private:
    void updateTexture(const PixelProcessingUnit& ppu, const MemoryBus& bus);
    void drawViewportOverlay(u8 scx, u8 scy);
    void drawInfoPanel(const PixelProcessingUnit& ppu);

    static constexpr u32 MapTiles = 32;
    static constexpr u32 MapPixels = 256; // 32 * 8
    static constexpr u32 ViewportWidth = 160;
    static constexpr u32 ViewportHeight = 144;
    static constexpr float PixelScale = 2.f;
    static constexpr u32 InfoPanelHeight = 80;
    static constexpr unsigned int WindowWidth = static_cast<unsigned int>(MapPixels * PixelScale);
    static constexpr unsigned int WindowHeight = InfoPanelHeight + static_cast<unsigned int>(MapPixels * PixelScale);

    static constexpr ImVec4 NameColor {0.55f, 0.75f, 1.f, 1.f};
    static constexpr ImVec4 ValueColor {1.f, 0.85f, 0.35f, 1.f};
    static constexpr ImVec4 EnabledColor {0.4f, 0.9f, 0.4f, 1.f};
    static constexpr ImVec4 DisabledColor {0.5f, 0.5f, 0.5f, 1.f};

    std::array<u8, MapPixels * MapPixels * 4> pixels{};
    sf::Texture mapTexture;
    sf::Sprite mapSprite;

    int selectedMap = 0;  // 0 = Background, 1 = Window
};
