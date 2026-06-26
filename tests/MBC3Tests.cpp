#include <doctest/doctest.h>

#include "CartridgeTestFixture.h"
#include "MBC/MBC3.h"

namespace
{
    constexpr u32 ROMBankSizeBytes = 0x4000;
    constexpr u32 RAMBankSizeBytes = 0x2000;
    constexpr u32 CyclesPerSecond = 4194304; // Mirrors MBC3::CyclesPerSecond

    void markBank(CartridgeTestFixture& fixture, u32 bankIndex, u8 marker)
    {
        fixture.setROMByte(bankIndex * ROMBankSizeBytes, marker);
    }

    // Advances the live clock by exactly one second.
    void tickOneSecond(MBC3& mbc)
    {
        for(u32 i = 0; i < CyclesPerSecond; ++i)
            mbc.tick();
    }

    void enableAndSelect(MBC3& mbc, u8 select)
    {
        mbc.write(0x0000, 0x0A);
        mbc.write(0x4000, select);
    }
}

TEST_CASE("MBC3 substitutes ROM bank 0 with bank 1 and masks by the available bank count")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(8 * ROMBankSizeBytes);
    markBank(fixture, 0, 0xAA);
    markBank(fixture, 1, 0xBB);
    markBank(fixture, 6, 0x66);

    MBC3 mbc(&fixture.getCartridge(), 8, 0, false, false);

    mbc.write(0x2000, 0x00); // substituted to bank 1
    CHECK(mbc.read(0x4000) == 0xBB);

    mbc.write(0x2000, 0x7E); // 0x7E & (8-1) = 6
    CHECK(mbc.read(0x4000) == 0x66);
}

TEST_CASE("MBC3 RAM-enable gates both cartridge RAM and RTC register access")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(2 * ROMBankSizeBytes);
    fixture.setRAMSize(RAMBankSizeBytes);

    MBC3 mbc(&fixture.getCartridge(), 2, 1, false, true);

    mbc.write(0x4000, 0x00); // select RAM bank 0
    mbc.write(0xA000, 0x42); // disabled, ignored
    CHECK(mbc.read(0xA000) == 0xFF);

    mbc.write(0x4000, 0x08); // select RTC Seconds
    mbc.write(0xA000, 0x05); // disabled, ignored
    CHECK(mbc.read(0xA000) == 0xFF);

    mbc.write(0x0000, 0x0A); // enable
    mbc.write(0x4000, 0x00);
    mbc.write(0xA000, 0x42);
    CHECK(mbc.read(0xA000) == 0x42);
}

TEST_CASE("MBC3 banked RAM keeps each bank's contents isolated")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(2 * ROMBankSizeBytes);
    fixture.setRAMSize(4 * RAMBankSizeBytes);

    MBC3 mbc(&fixture.getCartridge(), 2, 4, false, false);
    mbc.write(0x0000, 0x0A);

    mbc.write(0x4000, 0x01);
    mbc.write(0xA000, 0x11);

    mbc.write(0x4000, 0x02);
    mbc.write(0xA000, 0x22);

    mbc.write(0x4000, 0x01);
    CHECK(mbc.read(0xA000) == 0x11);

    mbc.write(0x4000, 0x02);
    CHECK(mbc.read(0xA000) == 0x22);
}

TEST_CASE("MBC3 reads return the latched RTC snapshot, not the live counters")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(2 * ROMBankSizeBytes);

    MBC3 mbc(&fixture.getCartridge(), 2, 0, false, true);
    enableAndSelect(mbc, 0x08); // Seconds

    mbc.write(0xA000, 10); // writes through to the live register
    CHECK(mbc.read(0xA000) == 0x00); // latched snapshot hasn't been refreshed yet

    mbc.write(0x6000, 0x00); // latch trigger: 0x00 then 0x01
    mbc.write(0x6000, 0x01);
    CHECK(mbc.read(0xA000) == 10); // now reflects the live value at latch time
}

TEST_CASE("MBC3 latch only triggers on an exact 0x00 -> 0x01 transition")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(2 * ROMBankSizeBytes);

    MBC3 mbc(&fixture.getCartridge(), 2, 0, false, true);
    enableAndSelect(mbc, 0x08);

    mbc.write(0xA000, 5);
    mbc.write(0x6000, 0x01); // no preceding 0x00, should not latch

    CHECK(mbc.read(0xA000) == 0x00);

    mbc.write(0x6000, 0x00);
    mbc.write(0x6000, 0x01); // proper transition
    CHECK(mbc.read(0xA000) == 5);
}

TEST_CASE("MBC3 halt flag stops the RTC from advancing")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(2 * ROMBankSizeBytes);

    MBC3 mbc(&fixture.getCartridge(), 2, 0, false, true);
    enableAndSelect(mbc, 0x0C); // Day-high (halt flag)
    mbc.write(0xA000, 0x40); // set halt bit

    tickOneSecond(mbc);

    enableAndSelect(mbc, 0x08);
    mbc.write(0x6000, 0x00);
    mbc.write(0x6000, 0x01);
    CHECK(mbc.read(0xA000) == 0); // still 0, the clock never advanced

    enableAndSelect(mbc, 0x0C);
    mbc.write(0xA000, 0x00); // clear halt bit
    tickOneSecond(mbc);

    enableAndSelect(mbc, 0x08);
    mbc.write(0x6000, 0x00);
    mbc.write(0x6000, 0x01);
    CHECK(mbc.read(0xA000) == 1); // resumed advancing
}

TEST_CASE("MBC3 RTC carries seconds into minutes, hours and the day counter")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(2 * ROMBankSizeBytes);

    MBC3 mbc(&fixture.getCartridge(), 2, 0, false, true);

    enableAndSelect(mbc, 0x08);
    mbc.write(0xA000, 59); // Seconds
    enableAndSelect(mbc, 0x09);
    mbc.write(0xA000, 59); // Minutes
    enableAndSelect(mbc, 0x0A);
    mbc.write(0xA000, 23); // Hours
    enableAndSelect(mbc, 0x0B);
    mbc.write(0xA000, 0); // Day low

    tickOneSecond(mbc);

    mbc.write(0x6000, 0x00);
    mbc.write(0x6000, 0x01);

    enableAndSelect(mbc, 0x08);
    CHECK(mbc.read(0xA000) == 0);
    enableAndSelect(mbc, 0x09);
    CHECK(mbc.read(0xA000) == 0);
    enableAndSelect(mbc, 0x0A);
    CHECK(mbc.read(0xA000) == 0);
    enableAndSelect(mbc, 0x0B);
    CHECK(mbc.read(0xA000) == 1); // day counter incremented
}

TEST_CASE("MBC3 day-counter overflow sets a sticky carry flag")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(2 * ROMBankSizeBytes);

    MBC3 mbc(&fixture.getCartridge(), 2, 0, false, true);

    enableAndSelect(mbc, 0x08);
    mbc.write(0xA000, 59);
    enableAndSelect(mbc, 0x09);
    mbc.write(0xA000, 59);
    enableAndSelect(mbc, 0x0A);
    mbc.write(0xA000, 23);
    enableAndSelect(mbc, 0x0B);
    mbc.write(0xA000, 0xFF); // day low = 0xFF
    enableAndSelect(mbc, 0x0C);
    mbc.write(0xA000, 0x01); // day high bit -> day counter = 0x1FF (511, the max)

    tickOneSecond(mbc); // overflows past day 511

    mbc.write(0x6000, 0x00);
    mbc.write(0x6000, 0x01);

    enableAndSelect(mbc, 0x0B);
    CHECK(mbc.read(0xA000) == 0); // day counter wrapped to 0
    enableAndSelect(mbc, 0x0C);
    u8 dayHigh = mbc.read(0xA000);
    CHECK((dayHigh & 0x80) != 0); // carry flag set
    CHECK((dayHigh & 0x01) == 0); // day MSB cleared

    // Sticky until manually cleared, even after another unrelated tick.
    tickOneSecond(mbc);
    enableAndSelect(mbc, 0x0C);
    mbc.write(0x6000, 0x00);
    mbc.write(0x6000, 0x01);
    CHECK((mbc.read(0xA000) & 0x80) != 0);

    mbc.write(0xA000, 0x00); // write DH with bit 7 clear -> manually clears the carry flag
    mbc.write(0x6000, 0x00);
    mbc.write(0x6000, 0x01);
    CHECK((mbc.read(0xA000) & 0x80) == 0);
}

TEST_CASE("MBC3 without a timer ignores RTC register selection entirely")
{
    CartridgeTestFixture fixture;
    fixture.setROMSize(2 * ROMBankSizeBytes);

    MBC3 mbc(&fixture.getCartridge(), 2, 0, false, false); // hasTimer = false
    enableAndSelect(mbc, 0x08);

    mbc.write(0xA000, 5); // no RTC hardware present, no-op
    CHECK(mbc.read(0xA000) == 0xFF);
}
