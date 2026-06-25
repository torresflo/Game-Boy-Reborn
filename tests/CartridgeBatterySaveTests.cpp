#include <doctest/doctest.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

#include "CartridgeTestFixture.h"

namespace
{
    // Builds a minimal-but-valid ROM file on disk with the given cartridge type/size
    // codes baked into the header at 0x100, and returns its path. romSizeCode follows
    // the same "bank count = 2 << code" convention as MemoryBankControllerFactoryTests.
    std::filesystem::path writeTestROM(const std::string& fileName, u8 cartridgeType, u8 romSizeCode, u8 ramSizeCode)
    {
        u32 romBankCount = 2u << romSizeCode;
        std::vector<u8> romData(romBankCount * 0x4000, 0);

        CartridgeHeader header{};
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

        return path;
    }

    std::vector<u8> readFile(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary);
        return std::vector<u8>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
}

TEST_CASE("Cartridge saveRAM writes battery-backed RAM to a .sav file next to the ROM")
{
    std::filesystem::path romPath = writeTestROM("gbr_test_save_basic.gb", 0x03, 0, 0x02); // MBC1+RAM+BATTERY, 8KB RAM

    CartridgeTestFixture fixture;
    REQUIRE(fixture.getCartridge().loadROM(romPath.string()));

    fixture.setRAMByte(0x0000, 0xAB);
    fixture.setRAMByte(0x1FFF, 0xCD);

    CHECK(fixture.getCartridge().saveRAM() == true);

    std::filesystem::path savePath = romPath;
    savePath.replace_extension(".sav");
    REQUIRE(std::filesystem::exists(savePath));

    std::vector<u8> saved = readFile(savePath);
    REQUIRE(saved.size() == 0x2000);
    CHECK(saved[0x0000] == 0xAB);
    CHECK(saved[0x1FFF] == 0xCD);
}

TEST_CASE("Cartridge saveRAM is a no-op for cartridges without a battery")
{
    std::filesystem::path romPath = writeTestROM("gbr_test_save_no_battery.gb", 0x08, 0, 0x02); // ROM+RAM, no battery

    CartridgeTestFixture fixture;
    REQUIRE(fixture.getCartridge().loadROM(romPath.string()));
    fixture.setRAMByte(0x0000, 0x99);

    CHECK(fixture.getCartridge().saveRAM() == false);

    std::filesystem::path savePath = romPath;
    savePath.replace_extension(".sav");
    CHECK_FALSE(std::filesystem::exists(savePath));
}

TEST_CASE("Cartridge restores battery-backed RAM saved by a previous load")
{
    std::filesystem::path romPath = writeTestROM("gbr_test_save_roundtrip.gb", 0x1B, 0, 0x03); // MBC5+RAM+BATTERY, 32KB RAM

    {
        CartridgeTestFixture writer;
        REQUIRE(writer.getCartridge().loadROM(romPath.string()));
        writer.setRAMByte(0x0042, 0x77);
        REQUIRE(writer.getCartridge().saveRAM() == true);
    }

    CartridgeTestFixture reader;
    REQUIRE(reader.getCartridge().loadROM(romPath.string()));
    CHECK(reader.getRAMByte(0x0042) == 0x77);
}

TEST_CASE("Cartridge ignores a save file whose size does not match the cartridge's RAM size")
{
    std::filesystem::path romPath = writeTestROM("gbr_test_save_mismatch.gb", 0x03, 0, 0x02); // expects 8KB RAM

    std::filesystem::path savePath = romPath;
    savePath.replace_extension(".sav");
    {
        std::ofstream badSave(savePath, std::ios::binary);
        std::vector<u8> wrongSizeContent(0x100, 0x77); // far smaller than the expected 8KB
        badSave.write(reinterpret_cast<const char*>(wrongSizeContent.data()), wrongSizeContent.size());
    }

    CartridgeTestFixture fixture;
    REQUIRE(fixture.getCartridge().loadROM(romPath.string()));

    CHECK(fixture.getRAMByte(0x0000) == 0x00); // mismatched save was ignored, RAM stays zero-initialized
}

TEST_CASE("Loading a new ROM flushes the previous cartridge's battery RAM first")
{
    std::filesystem::path romAPath = writeTestROM("gbr_test_save_switch_a.gb", 0x03, 0, 0x02); // MBC1+RAM+BATTERY
    std::filesystem::path romBPath = writeTestROM("gbr_test_save_switch_b.gb", 0x1B, 0, 0x02); // MBC5+RAM+BATTERY

    CartridgeTestFixture fixture;
    REQUIRE(fixture.getCartridge().loadROM(romAPath.string()));
    fixture.setRAMByte(0x0010, 0x55);

    REQUIRE(fixture.getCartridge().loadROM(romBPath.string())); // reuses the same Cartridge instance

    std::filesystem::path romASavePath = romAPath;
    romASavePath.replace_extension(".sav");
    REQUIRE(std::filesystem::exists(romASavePath));

    std::vector<u8> saved = readFile(romASavePath);
    REQUIRE(saved.size() == 0x2000);
    CHECK(saved[0x0010] == 0x55);
}
