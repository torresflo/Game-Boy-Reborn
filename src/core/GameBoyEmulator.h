#pragma once

#include <vector>

#include "Common.h"

#include "AudioProcessingUnit.h"
#include "Cartridge.h"
#include "CentralProcessingUnit.h"
#include "MemoryBus.h"
#include "PixelProcessingUnit.h"
#include "Gamepad.h"

class GameBoyEmulator
{
public:
    static constexpr u32 ClockFrequencyHz = 4194304;
    static constexpr u32 CyclesPerFrame = 70224; // LinesPerFrame * TicksPerLine, ie. ClockFrequencyHz / 59.7275 Hz.
    static constexpr double SecondsPerFrame = static_cast<double>(CyclesPerFrame) / ClockFrequencyHz;

    bool loadROM(std::string path);
    void stepOneFrame();
    bool saveRAM() const;

    bool isROMLoaded() const;
    bool isPaused() const;
    void setPaused(bool value);

    const Cartridge& getCartridge() const;
    const CentralProcessingUnit& getCPU() const;
    const PixelProcessingUnit& getPPU() const;
    const AudioProcessingUnit& getAPU() const;
    const MemoryBus& getMemoryBus() const;
    Gamepad& getGamepad();

    std::vector<s16> drainAudioSamples();

private:
    Cartridge cartridge;
    CentralProcessingUnit CPU;
    MemoryBus bus;
    PixelProcessingUnit PPU;
    AudioProcessingUnit APU;
    Gamepad gamepad;

    bool romLoaded = false;
    bool paused = false;
};