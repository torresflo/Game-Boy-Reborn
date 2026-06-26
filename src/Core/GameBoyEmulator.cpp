#include "GameBoyEmulator.h"

#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <type_traits>

#include "Save/SaveStateReader.h"
#include "Save/SaveStateWriter.h"

namespace
{
    struct SaveStateHeader
    {
        std::array<char, 8> magic;
        u32 formatVersion;
        char title[16];
        u8 headerChecksum;
        u16 globalChecksum;
    };
    static_assert(std::is_trivially_copyable_v<SaveStateHeader>);

    constexpr std::array<char, 8> SaveStateMagic = {'G', 'B', 'R', 'S', 'T', 'A', 'T', 'E'};
    constexpr u32 SaveStateFormatVersion = 1;
}

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

bool GameBoyEmulator::saveRAM() const
{
    return cartridge.saveRAM();
}

bool GameBoyEmulator::saveState(const std::string& path) const
{
    if(!romLoaded)
        return false;

    SaveStateHeader header{};
    header.magic = SaveStateMagic;
    header.formatVersion = SaveStateFormatVersion;
    std::memcpy(header.title, cartridge.getHeader().title, sizeof(header.title));
    header.headerChecksum = cartridge.getHeader().headerChecksum;
    header.globalChecksum = cartridge.getHeader().globalChecksum;

    SaveStateWriter writer;
    writer.write(header);

    CPU.serialize(writer);
    bus.serialize(writer);
    PPU.serialize(writer);
    APU.serialize(writer);
    cartridge.serialize(writer);
    gamepad.serialize(writer);

    std::ofstream file(path, std::ios::binary);
    if(!file.is_open())
    {
        Log::print(LogLevel::Error, "Cannot open save state file for writing: ", path);
        return false;
    }

    const std::vector<u8>& buffer = writer.getBuffer();
    file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    Log::print(LogLevel::Info, "Saved state to: ", path);
    return true;
}

bool GameBoyEmulator::loadState(const std::string& path)
{
    if(!romLoaded)
    {
        Log::print(LogLevel::Error, "Cannot load state: no ROM loaded");
        return false;
    }

    std::ifstream file(path, std::ios::binary);
    if(!file.is_open())
    {
        Log::print(LogLevel::Error, "Cannot open save state file: ", path);
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<u8> buffer(static_cast<size_t>(fileSize));
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    file.close();

    SaveStateReader reader(std::move(buffer));

    SaveStateHeader header{};
    if(!reader.read(header))
    {
        Log::print(LogLevel::Error, "Save state file is truncated: ", path);
        return false;
    }

    if(header.magic != SaveStateMagic)
    {
        Log::print(LogLevel::Error, "Not a valid save state file: ", path);
        return false;
    }

    if(header.formatVersion != SaveStateFormatVersion)
    {
        Log::print(LogLevel::Error, "Save state format version mismatch: ", path);
        return false;
    }

    const CartridgeHeader& currentHeader = cartridge.getHeader();
    if(std::memcmp(header.title, currentHeader.title, sizeof(header.title)) != 0
        || header.headerChecksum != currentHeader.headerChecksum
        || header.globalChecksum != currentHeader.globalChecksum)
    {
        Log::print(LogLevel::Error, "Save state does not match the currently loaded ROM: ", path);
        return false;
    }

    CPU.deserialize(reader);
    bus.deserialize(reader);
    PPU.deserialize(reader);
    APU.deserialize(reader);
    cartridge.deserialize(reader);
    gamepad.deserialize(reader);

    Log::print(LogLevel::Info, "Loaded state from: ", path);
    return true;
}

std::string GameBoyEmulator::getQuickSaveStatePath() const
{
    return std::filesystem::path(cartridge.getRomPath()).replace_extension(".gbstate").string();
}

bool GameBoyEmulator::quickSaveState() const
{
    return saveState(getQuickSaveStatePath());
}

bool GameBoyEmulator::quickLoadState()
{
    return loadState(getQuickSaveStatePath());
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
