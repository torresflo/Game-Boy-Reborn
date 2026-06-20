#pragma once

#include <array>

#include "Common.h"

class MemoryBus;

class PixelProcessingUnit
{
public:
    static constexpr u32 ScreenWidth = 160;
    static constexpr u32 ScreenHeight = 144;

    void initialize(MemoryBus* bus);
    void tick();

    const std::array<u32, ScreenWidth * ScreenHeight>& getFrameBuffer() const;

private:
    MemoryBus* memoryBus = nullptr;
    std::array<u32, ScreenWidth * ScreenHeight> frameBuffer{};
};