#pragma once

#include <array>

#include "Common.h"

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

    const std::array<u32, ScreenWidth * ScreenHeight>& getFrameBuffer() const;

    using Tile = std::array<u8, TileSize * TileSize>; 
    Tile decodeTileAtAddress(u16 tileAddress) const;
    Tile decodeTileAIndex(u32 tileIndex) const;

private:
    MemoryBus* memoryBus = nullptr;
    std::array<u32, ScreenWidth * ScreenHeight> frameBuffer{};
};