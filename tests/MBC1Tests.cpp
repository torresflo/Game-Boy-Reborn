#include <doctest/doctest.h>

#include "CartridgeTestFixture.h"
#include "mbc/MBC1.h"

namespace
{
    constexpr u32 ROMBankSizeBytes = 0x4000;
    constexpr u32 RAMBankSizeBytes = 0x2000;

    void markBank(CartridgeTestFixture& fixture, u32 bankIndex, u8 marker)
    {
        fixture.setROMByte(bankIndex * ROMBankSizeBytes, marker);
    }
}

TEST_CASE("MBC1 substitutes ROM bank 0 with bank 1 when selecting the switchable window")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(8 * ROMBankSizeBytes);
    markBank(fixture, 0, 0xAA);
    markBank(fixture, 1, 0xBB);

    MBC1 mbc(&fixture.getCartridge(), 8, 0, false);
    mbc.write(0x2000, 0x00); // request bank 0

    CHECK(mbc.read(0x4000) == 0xBB); // substituted to bank 1
}

TEST_CASE("MBC1 masks the ROM bank number by the available bank count")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(4 * ROMBankSizeBytes);
    markBank(fixture, 2, 0x12);

    MBC1 mbc(&fixture.getCartridge(), 4, 0, false);
    mbc.write(0x2000, 0x06); // 0x06 & (4-1) = 2

    CHECK(mbc.read(0x4000) == 0x12);
}

TEST_CASE("MBC1 gates cartridge RAM access behind the RAM-enable register")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(2 * ROMBankSizeBytes);
    fixture.setRAMSize(RAMBankSizeBytes);

    MBC1 mbc(&fixture.getCartridge(), 2, 1, false);

    mbc.write(0xA000, 0x42); // RAM disabled, write should be ignored
    CHECK(mbc.read(0xA000) == 0xFF);

    mbc.write(0x0000, 0x0A); // enable RAM
    mbc.write(0xA000, 0x42);
    CHECK(mbc.read(0xA000) == 0x42);

    mbc.write(0x0000, 0x00); // disable RAM again
    CHECK(mbc.read(0xA000) == 0xFF);
}

TEST_CASE("MBC1 simple mode keeps 0x0000-0x3FFF fixed at bank 0")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(64 * ROMBankSizeBytes);
    markBank(fixture, 0, 0xAA);
    markBank(fixture, 32, 0xCC);

    MBC1 mbc(&fixture.getCartridge(), 64, 0, false);
    mbc.write(0x4000, 0x01); // secondaryBankRegister = 1, simple mode still active

    CHECK(mbc.read(0x0000) == 0xAA); // still bank 0 in simple mode
}

TEST_CASE("MBC1 advanced mode switches the 0x0000-0x3FFF window via the secondary bank register")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(64 * ROMBankSizeBytes);
    markBank(fixture, 0, 0xAA);
    markBank(fixture, 32, 0xCC);

    MBC1 mbc(&fixture.getCartridge(), 64, 0, false);
    mbc.write(0x6000, 0x01); // advanced banking mode
    mbc.write(0x4000, 0x01); // secondaryBankRegister = 1 -> bank 32

    CHECK(mbc.read(0x0000) == 0xCC);
}

TEST_CASE("MBC1 RAM bank selection only takes effect in advanced mode")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(2 * ROMBankSizeBytes);
    fixture.setRAMSize(4 * RAMBankSizeBytes);

    MBC1 mbc(&fixture.getCartridge(), 2, 4, false);
    mbc.write(0x0000, 0x0A); // enable RAM

    // Simple mode: RAM access always targets bank 0, regardless of secondaryBankRegister.
    mbc.write(0x4000, 0x02); // secondaryBankRegister = 2
    mbc.write(0xA000, 0x11);
    CHECK(mbc.read(0xA000) == 0x11);

    // Switch to advanced mode: secondaryBankRegister now selects the RAM bank.
    mbc.write(0x6000, 0x01);
    mbc.write(0xA000, 0x22); // now writes to bank 2

    mbc.write(0x6000, 0x00); // back to simple mode, RAM access targets bank 0 again
    CHECK(mbc.read(0xA000) == 0x11); // bank 0's value, untouched by the advanced-mode write

    mbc.write(0x6000, 0x01); // advanced mode again
    CHECK(mbc.read(0xA000) == 0x22); // bank 2's value
}
