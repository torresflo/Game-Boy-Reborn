#include <doctest/doctest.h>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

#include "AudioProcessingUnit.h"
#include "Cartridge.h"
#include "CentralProcessingUnit.h"
#include "GameBoyEmulator.h"
#include "PixelProcessingUnit.h"

namespace
{
    // Mirrors tests/CartridgeBatterySaveTests.cpp's helper: a minimal-but-valid all-NOP ROM
    // (zero-filled past the header) with a given cartridge type/size and a distinguishing
    // title, so two ROMs built by this helper can be told apart by a save state's compatibility check.
    std::filesystem::path writeTestROM(const std::string& fileName, const std::string& title, u8 cartridgeType, u8 romSizeCode, u8 ramSizeCode)
    {
        u32 romBankCount = 2u << romSizeCode;
        std::vector<u8> romData(romBankCount * 0x4000, 0);

        CartridgeHeader header{};
        std::memcpy(header.title, title.c_str(), std::min(title.size(), sizeof(header.title)));
        header.cartridgeType = cartridgeType;
        header.romSize = romSizeCode;
        header.ramSize = ramSizeCode;
        std::memcpy(romData.data() + 0x100, &header, sizeof(CartridgeHeader));

        std::filesystem::path path = std::filesystem::temp_directory_path() / fileName;
        std::ofstream file(path, std::ios::binary);
        file.write(reinterpret_cast<const char*>(romData.data()), romData.size());
        file.close();

        std::filesystem::path savePath = path;
        savePath.replace_extension(".sav");
        std::filesystem::remove(savePath);

        std::filesystem::path statePath = path;
        statePath.replace_extension(".gbstate");
        std::filesystem::remove(statePath);

        return path;
    }

    bool registersEqual(const Registers& a, const Registers& b)
    {
        return a.A == b.A && a.F == b.F && a.B == b.B && a.C == b.C
            && a.D == b.D && a.E == b.E && a.H == b.H && a.L == b.L
            && a.SP == b.SP && a.PC == b.PC;
    }

    constexpr int FramesBeforeSave = 30;
    constexpr int FramesAfterSave = 30;
}

TEST_CASE("GameBoyEmulator saveState/loadState round-trip reproduces a later frame exactly")
{
    std::filesystem::path romPath = writeTestROM("gbr_test_state_roundtrip.gb", "STATETESTA", 0x03, 0, 0x02); // MBC1+RAM+BATTERY, 8KB RAM

    GameBoyEmulator emulatorA;
    REQUIRE(emulatorA.loadROM(romPath.string()));
    for(int i = 0; i < FramesBeforeSave; ++i)
        emulatorA.stepOneFrame();

    std::filesystem::path statePath = romPath;
    statePath.replace_extension(".gbstate");
    REQUIRE(emulatorA.saveState(statePath.string()));

    for(int i = 0; i < FramesAfterSave; ++i)
        emulatorA.stepOneFrame();

    GameBoyEmulator emulatorB;
    REQUIRE(emulatorB.loadROM(romPath.string()));
    for(int i = 0; i < FramesBeforeSave; ++i)
        emulatorB.stepOneFrame();

    REQUIRE(emulatorB.loadState(statePath.string()));

    for(int i = 0; i < FramesAfterSave; ++i)
        emulatorB.stepOneFrame();

    CHECK(registersEqual(emulatorA.getCPU().getRegisters(), emulatorB.getCPU().getRegisters()));
    CHECK(emulatorA.getCPU().getCycleCount() == emulatorB.getCPU().getCycleCount());
    CHECK(emulatorA.getPPU().getFrameBuffer() == emulatorB.getPPU().getFrameBuffer());
    CHECK(emulatorA.getAPU().getChannelEnabledMask() == emulatorB.getAPU().getChannelEnabledMask());

    std::filesystem::remove(statePath);
}

TEST_CASE("GameBoyEmulator loadState seeds a fresh emulator instead of merely matching one already converged")
{
    std::filesystem::path romPath = writeTestROM("gbr_test_state_seed_fresh.gb", "STATETESTA", 0x03, 0, 0x02);

    GameBoyEmulator emulatorA;
    REQUIRE(emulatorA.loadROM(romPath.string()));
    for(int i = 0; i < FramesBeforeSave; ++i)
        emulatorA.stepOneFrame();

    std::filesystem::path statePath = romPath;
    statePath.replace_extension(".gbstate");
    REQUIRE(emulatorA.saveState(statePath.string()));

    for(int i = 0; i < FramesAfterSave; ++i)
        emulatorA.stepOneFrame();

    GameBoyEmulator emulatorB;
    REQUIRE(emulatorB.loadROM(romPath.string()));
    REQUIRE(emulatorB.loadState(statePath.string())); // loaded immediately after ROM load, with no stepping first - this alone should seed it to the FramesBeforeSave mark

    for(int i = 0; i < FramesAfterSave; ++i)
        emulatorB.stepOneFrame();

    CHECK(registersEqual(emulatorA.getCPU().getRegisters(), emulatorB.getCPU().getRegisters()));
    CHECK(emulatorA.getCPU().getCycleCount() == emulatorB.getCPU().getCycleCount());
    CHECK(emulatorA.getPPU().getFrameBuffer() == emulatorB.getPPU().getFrameBuffer());

    std::filesystem::remove(statePath);
}

TEST_CASE("GameBoyEmulator loadState rejects a save state from a different ROM")
{
    std::filesystem::path romAPath = writeTestROM("gbr_test_state_mismatch_a.gb", "STATETESTA", 0x03, 0, 0x02);
    std::filesystem::path romBPath = writeTestROM("gbr_test_state_mismatch_b.gb", "STATETESTB", 0x03, 0, 0x02);

    GameBoyEmulator emulatorA;
    REQUIRE(emulatorA.loadROM(romAPath.string()));

    std::filesystem::path statePath = romAPath;
    statePath.replace_extension(".gbstate");
    REQUIRE(emulatorA.saveState(statePath.string()));

    GameBoyEmulator emulatorB;
    REQUIRE(emulatorB.loadROM(romBPath.string()));
    for(int i = 0; i < FramesBeforeSave; ++i)
        emulatorB.stepOneFrame();

    u64 cyclesBeforeLoad = emulatorB.getCPU().getCycleCount();
    CHECK_FALSE(emulatorB.loadState(statePath.string()));
    CHECK(emulatorB.getCPU().getCycleCount() == cyclesBeforeLoad);

    std::filesystem::remove(statePath);
}

TEST_CASE("GameBoyEmulator saveState/loadState are no-ops without a loaded ROM")
{
    std::filesystem::path statePath = std::filesystem::temp_directory_path() / "gbr_test_state_no_rom.gbstate";
    std::filesystem::remove(statePath);

    GameBoyEmulator emulator;
    CHECK_FALSE(emulator.isROMLoaded());

    CHECK_FALSE(emulator.saveState(statePath.string()));
    CHECK_FALSE(emulator.loadState(statePath.string()));
    CHECK_FALSE(std::filesystem::exists(statePath));
}

TEST_CASE("GameBoyEmulator saveState writes a plausibly-sized file")
{
    std::filesystem::path romPath = writeTestROM("gbr_test_state_filesize.gb", "STATETESTA", 0x03, 0, 0x02);

    GameBoyEmulator emulator;
    REQUIRE(emulator.loadROM(romPath.string()));
    emulator.stepOneFrame();

    std::filesystem::path statePath = romPath;
    statePath.replace_extension(".gbstate");
    REQUIRE(emulator.saveState(statePath.string()));

    REQUIRE(std::filesystem::exists(statePath));
    CHECK(std::filesystem::file_size(statePath) > PixelProcessingUnit::ScreenWidth * PixelProcessingUnit::ScreenHeight * sizeof(u32));

    std::filesystem::remove(statePath);
}

TEST_CASE("GameBoyEmulator quickSaveState/quickLoadState round-trip via the default .gbstate path next to the ROM")
{
    std::filesystem::path romPath = writeTestROM("gbr_test_state_quick.gb", "STATETESTA", 0x03, 0, 0x02);

    GameBoyEmulator emulator;
    REQUIRE(emulator.loadROM(romPath.string()));
    for(int i = 0; i < FramesBeforeSave; ++i)
        emulator.stepOneFrame();

    REQUIRE(emulator.quickSaveState());

    std::filesystem::path expectedPath = romPath;
    expectedPath.replace_extension(".gbstate");
    CHECK(emulator.getQuickSaveStatePath() == expectedPath.string());
    REQUIRE(std::filesystem::exists(expectedPath));

    Registers savedRegisters = emulator.getCPU().getRegisters();
    u64 savedCycles = emulator.getCPU().getCycleCount();

    for(int i = 0; i < FramesAfterSave; ++i)
        emulator.stepOneFrame();

    REQUIRE(emulator.quickLoadState());

    CHECK(registersEqual(emulator.getCPU().getRegisters(), savedRegisters));
    CHECK(emulator.getCPU().getCycleCount() == savedCycles);

    std::filesystem::remove(expectedPath);
}
