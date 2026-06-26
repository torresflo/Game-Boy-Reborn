#pragma once

#include <array>
#include <vector>

#include "Common.h"
#include "CentralProcessingUnitTypes.h"
#include "PixelProcessingUnitTypes.h"
#include "save/ISaveStateSerializable.h"

class MemoryBus;
class CentralProcessingUnit;

class PixelProcessingUnit : public ISaveStateSerializable
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
    static constexpr u32 MaxObjects = 10;        // Max objects selected per scanline
    static constexpr u32 MaxObjectsPerFetch = 3; // Bounds fetchEntryData's size; hardware has no such cap, but >3 objects overlapping one fetch window is rare
    static constexpr u8 ObjectScreenYOffset = 16;

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

    bool isWindowVisible() const;

    const std::array<u32, ScreenWidth * ScreenHeight>& getFrameBuffer() const;
    const std::array<u32, 4>& getObjectColors(u8 paletteNumber) const;

    using Tile = std::array<u8, TileSize * TileSize>;
    Tile decodeTileAtAddress(u16 tileAddress) const;
    Tile decodeTileAIndex(u32 tileIndex) const;

    virtual void serialize(SaveStateWriter& writer) const override;
    virtual void deserialize(SaveStateReader& reader) override;

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

    void processPixelFIFO();
    void updateFetchPixel();
    void pushPixelInBuffer();
    bool addFetchedDataInPixelFIFO();
    void resetPixelFIFO();

    s32 getObjectFIFOX(const ObjectAttributeMemoryEntry& object) const;
    void selectFetchedObjects();
    void loadObjectData(u8 offset);
    void loadLineObjects();
    u32 fetchObjectPixel(u32 currentColor, u8 backgroundColorIndex);

    bool isWindowActiveOnLine() const;
    void loadWindowTile();

    u32 currentFrame = 0;
    u32 lineTicks = 0;
    std::array<u32, ScreenWidth * ScreenHeight> frameBuffer{};

    PixelFIFOContext pixelFIFOContext;

    std::vector<ObjectAttributeMemoryEntry> lineObjects;
    std::vector<ObjectAttributeMemoryEntry> fetchedObjects;

    u8 windowLine = 0;

    LCDData LCD;
    MemoryBus* memoryBus = nullptr;
    CentralProcessingUnit* cpu = nullptr;
};