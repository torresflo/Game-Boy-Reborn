#include "Emulator.h"

namespace
{
    // 4.194304 MHz / 59.7275 Hz, the DMG's refresh rate.
    constexpr u32 CyclesPerFrame = 17556;
}

bool Emulator::loadROM(std::string path)
{
    if(!cartridge.loadROM(path))
        return false;

    Log::print(LogLevel::Info, "ROM loaded successfully");

    bus.initialize(&cartridge, &CPU);
    CPU.initialize(&bus);
    PPU.initialize(&bus);

    romLoaded = true;
    paused = false;

    return true;
}

void Emulator::stepOneFrame()
{
    u64 targetCycles = CPU.getCycleCount() + CyclesPerFrame;
    while(CPU.getCycleCount() < targetCycles)
        CPU.step();
}

bool Emulator::isROMLoaded() const
{
    return romLoaded;
}

bool Emulator::isPaused() const
{
    return paused;
}

void Emulator::setPaused(bool value)
{
    paused = value;
}

const Cartridge& Emulator::getCartridge() const
{
    return cartridge;
}

const CentralProcessingUnit& Emulator::getCPU() const
{
    return CPU;
}

const PixelProcessingUnit& Emulator::getPPU() const
{
    return PPU;
}
