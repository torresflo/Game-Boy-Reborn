#include "PixelProcessingUnit.h"

#include "MathUtils.h"
#include "MemoryBus.h"

namespace
{
    constexpr u16 TileDataStartAddress = 0x8000;
}

void PixelProcessingUnit::initialize(MemoryBus* bus)
{
    memoryBus = bus;
}

void PixelProcessingUnit::tick()
{
}

const std::array<u32, PixelProcessingUnit::ScreenWidth * PixelProcessingUnit::ScreenHeight>& PixelProcessingUnit::getFrameBuffer() const
{
    return frameBuffer;
}

PixelProcessingUnit::Tile PixelProcessingUnit::decodeTileAtAddress(u16 tileAddress) const
{
    Tile tile{};
    for(u32 row = 0; row < TileSize; ++row)
    {
        u8 lowByte = memoryBus->read(tileAddress + static_cast<u16>(row * 2));
        u8 highByte = memoryBus->read(tileAddress + static_cast<u16>(row * 2 + 1));

        for(u32 column = 0; column < TileSize; ++column)
        {
            u32 bit = 7 - column;
            tile[row * TileSize + column] = (MathUtils<u8>::getBitValue(highByte, bit) ? 2 : 0)
                                                    | (MathUtils<u8>::getBitValue(lowByte, bit) ? 1 : 0);
        }
    }

    return tile;
}

PixelProcessingUnit::Tile PixelProcessingUnit::decodeTileAIndex(u32 tileIndex) const
{
    u16 tileAddress = TileDataStartAddress + static_cast<u16>(tileIndex * PixelProcessingUnit::BytesPerTile);
    return decodeTileAtAddress(tileAddress);
}
