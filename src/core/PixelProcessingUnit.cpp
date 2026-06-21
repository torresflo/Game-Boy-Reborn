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

bool PixelProcessingUnit::getBackgroundEnabled() const
{
    u8 controlByte = memoryBus->read(LCDControlRegister);
    return MathUtils<u8>::getBitValue(controlByte, 0);
}

bool PixelProcessingUnit::getObjectEnabled() const
{
    u8 controlByte = memoryBus->read(LCDControlRegister);
    return MathUtils<u8>::getBitValue(controlByte, 1);
}

SpriteSize PixelProcessingUnit::getObjectSize() const
{
    u8 controlByte = memoryBus->read(LCDControlRegister);
    return MathUtils<u8>::getBitValue(controlByte, 2) ? SpriteSize::EightBySixteen : SpriteSize::EightByEight;
}

TileMapArea PixelProcessingUnit::getBackgroundTileMapArea() const
{
    u8 controlByte = memoryBus->read(LCDControlRegister);
    return MathUtils<u8>::getBitValue(controlByte, 3) ? TileMapArea::High : TileMapArea::Low;
}

TileDataArea PixelProcessingUnit::getTileDataArea() const
{
    u8 controlByte = memoryBus->read(LCDControlRegister);
    return MathUtils<u8>::getBitValue(controlByte, 4) ? TileDataArea::High : TileDataArea::Low;
}

bool PixelProcessingUnit::getWindowEnabled() const
{
    u8 controlByte = memoryBus->read(LCDControlRegister);
    return MathUtils<u8>::getBitValue(controlByte, 5);
}

TileMapArea PixelProcessingUnit::getWindowTileMapArea() const
{
    u8 controlByte = memoryBus->read(LCDControlRegister);
    return MathUtils<u8>::getBitValue(controlByte, 6) ? TileMapArea::High : TileMapArea::Low;
}

bool PixelProcessingUnit::getLCDEnabled() const
{
    u8 controlByte = memoryBus->read(LCDControlRegister);
    return MathUtils<u8>::getBitValue(controlByte, 7);
}

LCDMode PixelProcessingUnit::getLCDMode() const
{
    u8 statusByte = memoryBus->read(LCDStatusRegister);
    u8 modeValue = (MathUtils<u8>::getBitValue(statusByte, 1) ? 2 : 0)
                        | (MathUtils<u8>::getBitValue(statusByte, 0) ? 1 : 0);
    return static_cast<LCDMode>(modeValue);
}

void PixelProcessingUnit::setLCDMode(LCDMode mode)
{
    u8 statusByte = memoryBus->read(LCDStatusRegister);
    u8 modeValue = static_cast<u8>(mode);
    MathUtils<u8>::setBitValue(statusByte, 0, MathUtils<u8>::getBitValue(modeValue, 0));
    MathUtils<u8>::setBitValue(statusByte, 1, MathUtils<u8>::getBitValue(modeValue, 1));
    memoryBus->write(LCDStatusRegister, statusByte);
}

bool PixelProcessingUnit::getCoincidenceFlag() const
{
    u8 statusByte = memoryBus->read(LCDStatusRegister);
    return MathUtils<u8>::getBitValue(statusByte, 2);
}

void PixelProcessingUnit::setCoincidenceFlag(bool flag)
{
    u8 statusByte = memoryBus->read(LCDStatusRegister);
    MathUtils<u8>::setBitValue(statusByte, 2, flag);
    memoryBus->write(LCDStatusRegister, statusByte);
}

bool PixelProcessingUnit::getHorizontalBlankInterruptEnabled() const
{
    u8 statusByte = memoryBus->read(LCDStatusRegister);
    return MathUtils<u8>::getBitValue(statusByte, 3);
}

void PixelProcessingUnit::setHorizontalBlankInterruptEnabled(bool enabled)
{
    u8 statusByte = memoryBus->read(LCDStatusRegister);
    MathUtils<u8>::setBitValue(statusByte, 3, enabled);
    memoryBus->write(LCDStatusRegister, statusByte);
}

bool PixelProcessingUnit::getVerticalBlankInterruptEnabled() const
{
    u8 statusByte = memoryBus->read(LCDStatusRegister);
    return MathUtils<u8>::getBitValue(statusByte, 4);
}

void PixelProcessingUnit::setVerticalBlankInterruptEnabled(bool enabled)
{
    u8 statusByte = memoryBus->read(LCDStatusRegister);
    MathUtils<u8>::setBitValue(statusByte, 4, enabled);
    memoryBus->write(LCDStatusRegister, statusByte);
}

bool PixelProcessingUnit::getObjectAccessMemoryInterruptEnabled() const
{
    u8 statusByte = memoryBus->read(LCDStatusRegister);
    return MathUtils<u8>::getBitValue(statusByte, 5);
}

void PixelProcessingUnit::setObjectAccessMemoryInterruptEnabled(bool enabled)
{
    u8 statusByte = memoryBus->read(LCDStatusRegister);
    MathUtils<u8>::setBitValue(statusByte, 5, enabled);
    memoryBus->write(LCDStatusRegister, statusByte);
}

bool PixelProcessingUnit::getLYCInterruptEnabled() const
{
    u8 statusByte = memoryBus->read(LCDStatusRegister);
    return MathUtils<u8>::getBitValue(statusByte, 6);
}

void PixelProcessingUnit::setLYCInterruptEnabled(bool enabled)
{
    u8 statusByte = memoryBus->read(LCDStatusRegister);
    MathUtils<u8>::setBitValue(statusByte, 6, enabled);
    memoryBus->write(LCDStatusRegister, statusByte);
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
