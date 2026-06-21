#pragma once

#include <array>

#include "Common.h"
#include "PixelProcessingUnitTypes.h"

class MemoryBus;

class PixelProcessingUnit
{
public:
    static constexpr u32 ScreenWidth = 160;
    static constexpr u32 ScreenHeight = 144;
    static constexpr u32 TileSize = 8;
    static constexpr u32 BytesPerTile = 16;
    static constexpr u32 TileColumns = 16;
    static constexpr u32 TileRows = 24; // 16 x 24 = 384 tiles

    void initialize(MemoryBus* bus);
    void tick();

    //Control byte (LCDC - 0xFF40)
    bool getBackgroundEnabled() const;
    bool getObjectEnabled() const;
    SpriteSize getObjectSize() const;
    TileMapArea getBackgroundTileMapArea() const;
    TileDataArea getTileDataArea() const;
    bool getWindowEnabled() const;
    TileMapArea getWindowTileMapArea() const;
    bool getLCDEnabled() const;

    //Status byte (STAT - 0xFF41)
    LCDMode getLCDMode() const;
    void setLCDMode(LCDMode mode);
    bool getCoincidenceFlag() const;
    void setCoincidenceFlag(bool flag);
    bool getHorizontalBlankInterruptEnabled() const;
    void setHorizontalBlankInterruptEnabled(bool enabled);
    bool getVerticalBlankInterruptEnabled() const;
    void setVerticalBlankInterruptEnabled(bool enabled);
    bool getObjectAccessMemoryInterruptEnabled() const;
    void setObjectAccessMemoryInterruptEnabled(bool enabled);
    bool getLYCInterruptEnabled() const;
    void setLYCInterruptEnabled(bool enabled);

    const std::array<u32, ScreenWidth * ScreenHeight>& getFrameBuffer() const;

    using Tile = std::array<u8, TileSize * TileSize>; 
    Tile decodeTileAtAddress(u16 tileAddress) const;
    Tile decodeTileAIndex(u32 tileIndex) const;

private:
    static constexpr u16 LCDControlRegister = 0xFF40;
    static constexpr u16 LCDStatusRegister = 0xFF41;

    MemoryBus* memoryBus = nullptr;
    std::array<u32, ScreenWidth * ScreenHeight> frameBuffer{};
};