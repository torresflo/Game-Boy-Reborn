#include "GameBoyEmulator.h"

bool GameBoyEmulator::loadROM(std::string path)
{
    if(!cartridge.loadROM(path))
        return false;

    Log::print(LogLevel::Info, "ROM loaded successfully");

    bus.initialize(&cartridge, &CPU, &PPU, &APU, &gamepad);
    CPU.initialize(&bus, &PPU, &APU);
    PPU.initialize(&bus, &CPU);
    APU.initialize();
    gamepad.initialize();

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

const AudioProcessingUnit& GameBoyEmulator::getAPU() const
{
    return APU;
}

std::vector<s16> GameBoyEmulator::drainAudioSamples()
{
    return APU.drainSampleBuffer();
}

const MemoryBus& GameBoyEmulator::getMemoryBus() const
{
    return bus;
}

Gamepad& GameBoyEmulator::getGamepad()
{
    return gamepad;
}
