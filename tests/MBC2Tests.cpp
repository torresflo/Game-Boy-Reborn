#include <doctest/doctest.h>

#include "CartridgeTestFixture.h"
#include "mbc/MBC2.h"

namespace
{
    constexpr u32 ROMBankSizeBytes = 0x4000;
    constexpr u32 BuiltInRAMSizeBytes = 0x200;

    void markBank(CartridgeTestFixture& fixture, u32 bankIndex, u8 marker)
    {
        fixture.setROMByte(bankIndex * ROMBankSizeBytes, marker);
    }
}

TEST_CASE("MBC2 substitutes ROM bank 0 with bank 1")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(4 * ROMBankSizeBytes);
    markBank(fixture, 0, 0xAA);
    markBank(fixture, 1, 0xBB);

    MBC2 mbc(&fixture.getCartridge(), 4, false);
    mbc.write(0x2100, 0x00); // address bit 8 set -> ROM bank select, bank 0 substituted to 1

    CHECK(mbc.read(0x4000) == 0xBB);
}

TEST_CASE("MBC2 disambiguates RAM-enable from ROM-bank-select by address bit 8, not by range")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(4 * ROMBankSizeBytes);
    fixture.setRAMSize(BuiltInRAMSizeBytes);

    MBC2 mbc(&fixture.getCartridge(), 4, false);

    // Bit 8 set: interpreted as a ROM bank write, NOT RAM-enable, even though
    // the low nibble of the value (0x0A) matches the RAM-enable pattern.
    mbc.write(0x2100, 0x0A);
    mbc.write(0xA000, 0x05);
    CHECK(mbc.read(0xA000) == 0xFF); // RAM still disabled

    // Bit 8 clear: this is the RAM-enable register.
    mbc.write(0x0000, 0x0A);
    mbc.write(0xA000, 0x05);
    CHECK(mbc.read(0xA000) == 0xF5);
}

TEST_CASE("MBC2 only stores the low nibble of RAM writes and reads the upper nibble as set")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(2 * ROMBankSizeBytes);
    fixture.setRAMSize(BuiltInRAMSizeBytes);

    MBC2 mbc(&fixture.getCartridge(), 2, false);
    mbc.write(0x0000, 0x0A); // enable RAM

    mbc.write(0xA000, 0xFF);
    CHECK(mbc.read(0xA000) == 0xFF); // low nibble 0xF, upper forced high

    mbc.write(0xA000, 0x05);
    CHECK(mbc.read(0xA000) == 0xF5);
}

TEST_CASE("MBC2's built-in RAM mirrors across the whole 0xA000-0xBFFF window")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(2 * ROMBankSizeBytes);
    fixture.setRAMSize(BuiltInRAMSizeBytes);

    MBC2 mbc(&fixture.getCartridge(), 2, false);
    mbc.write(0x0000, 0x0A); // enable RAM

    mbc.write(0xA001, 0x07);

    CHECK(mbc.read(0xA201) == 0xF7); // offset 1, mirrored 0x200 bytes later
    CHECK(mbc.read(0xBE01) == 0xF7); // offset 1, mirrored across the last mirror as well
}
