#pragma once

#include "Common.h"

#include "Cartridge.h"
#include "CentralProcessingUnit.h"
#include "MemoryBus.h"
#include "PixelProcessingUnit.h"
#include "Timer.h"

/**
 * Emulator components : Cart, CPU, Address Bus, PPU (Pixel-Processing Unit), Timer
 */

class Emulator
{
public:
    void run(std::string path);

private:
    void delay(u32 ms);

    Cartridge cartridge;
    CentralProcessingUnit CPU;
    MemoryBus bus;
    PixelProcessingUnit PPU;

    bool paused = false;
    bool running = false;
};