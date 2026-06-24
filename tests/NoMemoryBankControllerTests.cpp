#include <doctest/doctest.h>

#include "CartridgeTestFixture.h"
#include "mbc/NoMemoryBankController.h"

TEST_CASE("NoMemoryBankController with no RAM always reads 0xFF and ignores writes")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(0x8000);
    fixture.setROMByte(0x0000, 0x42);

    NoMemoryBankController mbc(&fixture.getCartridge(), false, 0);

    CHECK(mbc.read(0x0000) == 0x42);
    mbc.write(0xA000, 0x99); // no RAM, no-op
    CHECK(mbc.read(0xA000) == 0xFF);
}

TEST_CASE("NoMemoryBankController with fixed RAM reads and writes with no enable gating")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(0x8000);
    fixture.setRAMSize(0x2000);

    NoMemoryBankController mbc(&fixture.getCartridge(), false, 0x2000);

    mbc.write(0xA000, 0x55); // no enable register exists on this cartridge type
    CHECK(mbc.read(0xA000) == 0x55);
}

TEST_CASE("NoMemoryBankController falls back to 0xFF beyond the allocated RAM size")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(0x8000);
    fixture.setRAMSize(0x800); // a 2KB chip, smaller than the full 8KB window

    NoMemoryBankController mbc(&fixture.getCartridge(), false, 0x800);

    mbc.write(0xA000, 0x11);
    CHECK(mbc.read(0xA000) == 0x11);

    CHECK(mbc.read(0xA800) == 0xFF); // beyond the 0x800-byte chip
    mbc.write(0xA800, 0x22); // no-op
    CHECK(mbc.read(0xA800) == 0xFF);
}
