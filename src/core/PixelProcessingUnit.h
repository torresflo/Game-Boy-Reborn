#pragma once

#include <array>

#include "Common.h"
#include "CentralProcessingUnitTypes.h"
#include "PixelProcessingUnitTypes.h"

class MemoryBus;
class CentralProcessingUnit;

class PixelProcessingUnit
{
public:
    static constexpr u32 LinesPerFrame = 154;
    static constexpr u32 TicksPerLine = 456;
    static constexpr u32 ScreenWidth = 160; // X
    static constexpr u32 ScreenHeight = 144;// Y
    static constexpr u32 TileSize = 8;
    static constexpr u32 BytesPerTile = 16;
    static constexpr u32 TileColumns = 16;
    static constexpr u32 TileRows = 24;     // 16 x 24 = 384 tiles

    void initialize(MemoryBus* bus, CentralProcessingUnit* cpuPtr);
    void tick();

    //Memory-mapped register access (0xFF40-0xFF4B, except 0xFF46/DMA which MemoryBus owns)
    u8 readRegister(u16 address) const;
    void writeRegister(u16 address, u8 value);

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
    bool getCoincidenceFlag() const;
    bool getHorizontalBlankInterruptEnabled() const;
    bool getVerticalBlankInterruptEnabled() const;
    bool getObjectAccessMemoryInterruptEnabled() const;
    bool getLYCInterruptEnabled() const;

    const std::array<u32, ScreenWidth * ScreenHeight>& getFrameBuffer() const;

    using Tile = std::array<u8, TileSize * TileSize>;
    Tile decodeTileAtAddress(u16 tileAddress) const;
    Tile decodeTileAIndex(u32 tileIndex) const;

private:
    void updateHorizontalBlankMode();
    void updateVerticalBlankMode();
    void updateObjectAccessMemoryScanMode();
    void updatePixelDrawingMode();

    void setLCDMode(LCDMode mode);
    void setCoincidenceFlag(bool flag);
    void setHorizontalBlankInterruptEnabled(bool enabled);
    void setVerticalBlankInterruptEnabled(bool enabled);
    void setObjectAccessMemoryInterruptEnabled(bool enabled);
    void setLYCInterruptEnabled(bool enabled);

    void incrementCoordinateY();
    void resetCoordinateY();

    u32 currentFrame;
    u32 lineTicks;
    std::array<u32, ScreenWidth * ScreenHeight> frameBuffer{};

    u64 previousFrameTime = 0;
    u64 startTimer = 0;
    u64 frameCount = 0;

    LCDData LCD;
    MemoryBus* memoryBus = nullptr;
    CentralProcessingUnit* cpu = nullptr;
};