#include "PixelProcessingUnit.h"

#include "MemoryBus.h"

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
