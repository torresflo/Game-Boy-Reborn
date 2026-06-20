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
    bool loadROM(std::string path);
    void stepOneFrame();

    bool isROMLoaded() const;
    bool isPaused() const;
    void setPaused(bool value);

    const Cartridge& getCartridge() const;
    const CentralProcessingUnit& getCPU() const;
    const PixelProcessingUnit& getPPU() const;

private:
    Cartridge cartridge;
    CentralProcessingUnit CPU;
    MemoryBus bus;
    PixelProcessingUnit PPU;

    bool romLoaded = false;
    bool paused = false;
};