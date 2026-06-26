#include <doctest/doctest.h>

#include "CartridgeTestFixture.h"
#include "MBC/MBC5.h"

namespace
{
    constexpr u32 ROMBankSizeBytes = 0x4000;
    constexpr u32 RAMBankSizeBytes = 0x2000;

    void markBank(CartridgeTestFixture& fixture, u32 bankIndex, u8 marker)
    {
        fixture.setROMByte(bankIndex * ROMBankSizeBytes, marker);
    }
}

TEST_CASE("MBC5 keeps ROM bank 0 selectable in the switchable window, unlike MBC1/MBC3")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(4 * ROMBankSizeBytes);
    markBank(fixture, 0, 0xAA);
    markBank(fixture, 1, 0xBB);

    MBC5 mbc(&fixture.getCartridge(), 4, 0, false, false);
    mbc.write(0x2000, 0x00); // low byte
    mbc.write(0x3000, 0x00); // high bit

    CHECK(mbc.read(0x4000) == 0xAA); // bank 0, not substituted
}

TEST_CASE("MBC5 splits the 9-bit ROM bank number across the two write registers")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(257 * ROMBankSizeBytes); // enough to cover bank 256
    markBank(fixture, 0, 0xAA);
    markBank(fixture, 256, 0xCC); // bank 256 = 0x100, requires bit 8

    MBC5 mbc(&fixture.getCartridge(), 512, 0, false, false);
    mbc.write(0x2000, 0x00); // low byte = 0
    mbc.write(0x3000, 0x01); // bit 8 set -> bank 256

    CHECK(mbc.read(0x4000) == 0xCC);
}

TEST_CASE("MBC5 gates cartridge RAM access behind the RAM-enable register")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(2 * ROMBankSizeBytes);
    fixture.setRAMSize(RAMBankSizeBytes);

    MBC5 mbc(&fixture.getCartridge(), 2, 1, false, false);

    mbc.write(0xA000, 0x42);
    CHECK(mbc.read(0xA000) == 0xFF);

    mbc.write(0x0000, 0x0A);
    mbc.write(0xA000, 0x42);
    CHECK(mbc.read(0xA000) == 0x42);
}

TEST_CASE("MBC5 rumble carts exclude bit 3 of the RAM bank register from the bank index")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(2 * ROMBankSizeBytes);
    fixture.setRAMSize(4 * RAMBankSizeBytes); // RAMBankCount = 4, needs only 2 bits

    MBC5 mbc(&fixture.getCartridge(), 2, 4, false, true); // hasRumble = true
    mbc.write(0x0000, 0x0A);

    mbc.write(0x4000, 0x0B); // bit3 (motor) set, bits0-2 = 3
    mbc.write(0xA000, 0x77);

    mbc.write(0x4000, 0x03); // motor bit clear, same low bits -> same bank
    CHECK(mbc.read(0xA000) == 0x77);
}

TEST_CASE("MBC5 non-rumble carts use all 4 bits of the RAM bank register")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(2 * ROMBankSizeBytes);
    fixture.setRAMSize(16 * RAMBankSizeBytes); // RAMBankCount = 16, needs all 4 bits

    MBC5 mbc(&fixture.getCartridge(), 2, 16, false, false); // hasRumble = false
    mbc.write(0x0000, 0x0A);

    mbc.write(0x4000, 0x0B); // bank 11
    mbc.write(0xA000, 0x99);

    mbc.write(0x4000, 0x03); // bank 3, distinct from bank 11 without rumble masking
    CHECK(mbc.read(0xA000) == 0x00);
}
