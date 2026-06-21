#include "GameBoyEmulator.h"

namespace
{
    // 4.194304 MHz / 59.7275 Hz, the DMG's refresh rate.
    constexpr u32 CyclesPerFrame = 17556;
}

bool GameBoyEmulator::loadROM(std::string path)
{
    if(!cartridge.loadROM(path))
        return false;

    Log::print(LogLevel::Info, "ROM loaded successfully");

    bus.initialize(&cartridge, &CPU, &PPU);
    CPU.initialize(&bus, &PPU);
    PPU.initialize(&bus, &CPU);

    romLoaded = true;
    paused = false;

    return true;
}

void GameBoyEmulator::stepOneFrame()
{
    u64 targetCycles = CPU.getCycleCount() + CyclesPerFrame;
    while(CPU.getCycleCount() < targetCycles)
        CPU.step();
}

bool GameBoyEmulator::isROMLoaded() const
{
    return romLoaded;
}

bool GameBoyEmulator::isPaused() const
{
    return paused;
}

void GameBoyEmulator::setPaused(bool value)
{
    paused = value;
}

const Cartridge& GameBoyEmulator::getCartridge() const
{
    return cartridge;
}

const CentralProcessingUnit& GameBoyEmulator::getCPU() const
{
    return CPU;
}

const PixelProcessingUnit& GameBoyEmulator::getPPU() const
{
    return PPU;
}

const MemoryBus& GameBoyEmulator::getMemoryBus() const
{
    return bus;
}
