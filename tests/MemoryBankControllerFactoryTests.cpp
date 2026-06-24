#include <doctest/doctest.h>

#include "CartridgeTestFixture.h"
#include "mbc/MemoryBankControllerFactory.h"
#include "mbc/NoMemoryBankController.h"
#include "mbc/MBC1.h"
#include "mbc/MBC2.h"
#include "mbc/MBC3.h"
#include "mbc/MBC5.h"

namespace
{
    // header.romSize code -> bank count is 2 << code; size the ROM buffer to match.
    void setupROM(CartridgeTestFixture& fixture, u8 cartridgeType, u8 romSizeCode, u8 ramSizeCode)
    {
        u32 romBankCount = 2u << romSizeCode;
        fixture.setROMSize(romBankCount * 0x4000);
        fixture.setHeader(cartridgeType, romSizeCode, ramSizeCode);
    }
}

TEST_CASE("Factory creates a non-battery NoMemoryBankController for ROM ONLY")
{
    CartridgeTestFixture fixture;
    setupROM(fixture, 0x00, 0, 0x00);

    auto mbc = MemoryBankControllerFactory::create(fixture.getCartridge());

    REQUIRE(mbc != nullptr);
    CHECK(dynamic_cast<NoMemoryBankController*>(mbc.get()) != nullptr);
    CHECK(mbc->hasBattery() == false);
}

TEST_CASE("Factory creates NoMemoryBankController with the right battery flag for ROM+RAM types")
{
    CartridgeTestFixture romRAM;
    setupROM(romRAM, 0x08, 0, 0x02);
    auto romRAMMbc = MemoryBankControllerFactory::create(romRAM.getCartridge());
    REQUIRE(romRAMMbc != nullptr);
    CHECK(dynamic_cast<NoMemoryBankController*>(romRAMMbc.get()) != nullptr);
    CHECK(romRAMMbc->hasBattery() == false);
    CHECK(romRAM.getRAMSize() == 0x2000);

    CartridgeTestFixture romRAMBattery;
    setupROM(romRAMBattery, 0x09, 0, 0x02);
    auto romRAMBatteryMbc = MemoryBankControllerFactory::create(romRAMBattery.getCartridge());
    REQUIRE(romRAMBatteryMbc != nullptr);
    CHECK(dynamic_cast<NoMemoryBankController*>(romRAMBatteryMbc.get()) != nullptr);
    CHECK(romRAMBatteryMbc->hasBattery() == true);
}

TEST_CASE("Factory creates MBC1 with the right battery flag")
{
    CartridgeTestFixture fixture;
    setupROM(fixture, 0x03, 0, 0x02);

    auto mbc = MemoryBankControllerFactory::create(fixture.getCartridge());

    REQUIRE(mbc != nullptr);
    CHECK(dynamic_cast<MBC1*>(mbc.get()) != nullptr);
    CHECK(mbc->hasBattery() == true);
}

TEST_CASE("Factory creates MBC2 and forces RAM size to 512 bytes regardless of the header")
{
    CartridgeTestFixture fixture;
    setupROM(fixture, 0x06, 0, 0x03); // header claims 32KB RAM, must be ignored

    auto mbc = MemoryBankControllerFactory::create(fixture.getCartridge());

    REQUIRE(mbc != nullptr);
    CHECK(dynamic_cast<MBC2*>(mbc.get()) != nullptr);
    CHECK(mbc->hasBattery() == true);
    CHECK(fixture.getRAMSize() == 0x200);
}

TEST_CASE("Factory sets the battery flag correctly across all MBC3 cartridge types")
{
    struct Case { u8 type; bool expectedBattery; };
    const Case cases[] = { {0x0F, true}, {0x10, true}, {0x11, false}, {0x12, false}, {0x13, true} };

    for(const auto& testCase : cases)
    {
        CartridgeTestFixture fixture;
        setupROM(fixture, testCase.type, 0, 0x00);
        auto mbc = MemoryBankControllerFactory::create(fixture.getCartridge());
        REQUIRE(mbc != nullptr);
        CHECK(dynamic_cast<MBC3*>(mbc.get()) != nullptr);
        CHECK(mbc->hasBattery() == testCase.expectedBattery);
    }
}

TEST_CASE("Factory only wires up RTC register access for MBC3 TIMER cartridge types")
{
    CartridgeTestFixture timerFixture;
    setupROM(timerFixture, 0x0F, 0, 0x00);
    auto timerMbc = MemoryBankControllerFactory::create(timerFixture.getCartridge());
    REQUIRE(timerMbc != nullptr);
    timerMbc->write(0x0000, 0x0A);
    timerMbc->write(0x4000, 0x08); // select RTC Seconds register
    CHECK(timerMbc->read(0xA000) == 0x00); // a defined RTC value, not 0xFF

    CartridgeTestFixture plainFixture;
    setupROM(plainFixture, 0x11, 0, 0x00);
    auto plainMbc = MemoryBankControllerFactory::create(plainFixture.getCartridge());
    REQUIRE(plainMbc != nullptr);
    plainMbc->write(0x0000, 0x0A);
    plainMbc->write(0x4000, 0x08); // not a valid RAM bank and no RTC hardware present
    CHECK(plainMbc->read(0xA000) == 0xFF);
}

TEST_CASE("Factory sets battery flag and creates MBC5 across all MBC5 cartridge types")
{
    struct Case { u8 type; bool expectedBattery; };
    const Case cases[] =
    {
        {0x19, false}, {0x1A, false}, {0x1B, true},
        {0x1C, false}, {0x1D, false}, {0x1E, true}
    };

    for(const auto& testCase : cases)
    {
        CartridgeTestFixture fixture;
        setupROM(fixture, testCase.type, 0, 0x02);
        auto mbc = MemoryBankControllerFactory::create(fixture.getCartridge());
        REQUIRE(mbc != nullptr);
        CHECK(dynamic_cast<MBC5*>(mbc.get()) != nullptr);
        CHECK(mbc->hasBattery() == testCase.expectedBattery);
    }
}

TEST_CASE("Factory returns nullptr for an unsupported cartridge type")
{
    CartridgeTestFixture fixture;
    setupROM(fixture, 0x20, 0, 0x00); // MBC6, out of scope

    auto mbc = MemoryBankControllerFactory::create(fixture.getCartridge());

    CHECK(mbc == nullptr);
}
