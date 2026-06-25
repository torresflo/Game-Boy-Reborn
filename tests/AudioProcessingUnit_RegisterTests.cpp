#include <doctest/doctest.h>

#include "AudioProcessingUnit.h"

TEST_CASE("NR10 always reads bit 7 as set")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF10, 0x00);
    CHECK(apu.readRegister(0xFF10) == 0x80);

    apu.writeRegister(0xFF10, 0x7F);
    CHECK(apu.readRegister(0xFF10) == 0xFF);
}

TEST_CASE("NR11 length bits are write-only and read back as 1")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF11, 0x80); // duty 10, length 0
    CHECK(apu.readRegister(0xFF11) == 0xBF);

    apu.writeRegister(0xFF11, 0x3F); // duty 00, length 63
    CHECK(apu.readRegister(0xFF11) == 0x3F);
}

TEST_CASE("NR12 is fully readable and round-trips")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF12, 0xF3);
    CHECK(apu.readRegister(0xFF12) == 0xF3);
}

TEST_CASE("NR13 and NR23 and NR33 are write-only and always read 0xFF")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF13, 0x42);
    CHECK(apu.readRegister(0xFF13) == 0xFF);

    apu.writeRegister(0xFF18, 0x42);
    CHECK(apu.readRegister(0xFF18) == 0xFF);

    apu.writeRegister(0xFF1D, 0x42);
    CHECK(apu.readRegister(0xFF1D) == 0xFF);
}

TEST_CASE("NR14 unused and period-high bits always read 1, trigger always reads 1, length enable round-trips")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF14, 0x00);
    CHECK(apu.readRegister(0xFF14) == 0xBF);

    apu.writeRegister(0xFF14, 0xFF); // includes trigger bit
    CHECK(apu.readRegister(0xFF14) == 0xFF);
}

TEST_CASE("NR21-NR24 mirror NR11-NR14's masking on channel 2")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF16, 0x80);
    CHECK(apu.readRegister(0xFF16) == 0xBF);

    apu.writeRegister(0xFF19, 0x40);
    CHECK(apu.readRegister(0xFF19) == 0xFF); // length enable set, rest forced to 1
}

TEST_CASE("NR30 bit 7 is the only readable bit, rest read as 1")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF1A, 0x00);
    CHECK(apu.readRegister(0xFF1A) == 0x7F);

    apu.writeRegister(0xFF1A, 0x80);
    CHECK(apu.readRegister(0xFF1A) == 0xFF);
}

TEST_CASE("NR31 and NR41 are fully write-only and always read 0xFF")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF1B, 0x99);
    CHECK(apu.readRegister(0xFF1B) == 0xFF);

    apu.writeRegister(0xFF20, 0x99);
    CHECK(apu.readRegister(0xFF20) == 0xFF);
}

TEST_CASE("NR32 only exposes the output level bits, rest read as 1")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF1C, 0x60); // output level = 11
    CHECK(apu.readRegister(0xFF1C) == 0xFF);

    apu.writeRegister(0xFF1C, 0x00);
    CHECK(apu.readRegister(0xFF1C) == 0x9F);
}

TEST_CASE("NR34 mirrors NR14's masking on channel 3")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF1E, 0x40);
    CHECK(apu.readRegister(0xFF1E) == 0xFF);
}

TEST_CASE("NR42 and NR43 are fully readable on channel 4")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF21, 0xF3);
    CHECK(apu.readRegister(0xFF21) == 0xF3);

    apu.writeRegister(0xFF22, 0x55);
    CHECK(apu.readRegister(0xFF22) == 0x55);
}

TEST_CASE("NR44 mirrors NR14's masking on channel 4")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF23, 0x00);
    CHECK(apu.readRegister(0xFF23) == 0xBF);
}

TEST_CASE("NR50 and NR51 are fully readable and round-trip")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF24, 0xA7);
    CHECK(apu.readRegister(0xFF24) == 0xA7);

    apu.writeRegister(0xFF25, 0xC3);
    CHECK(apu.readRegister(0xFF25) == 0xC3);
}

TEST_CASE("NR52 unused bits always read 1 and the master power bit round-trips")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF26, 0x00);
    CHECK(apu.readRegister(0xFF26) == 0x70);

    apu.writeRegister(0xFF26, 0x80);
    CHECK(apu.readRegister(0xFF26) == 0xF0);
}

TEST_CASE("NR52 status bits reflect live channel-enabled state and ignore writes to bits 3-0")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF12, 0xF0); // NR12: volume 15 -> DAC enabled
    apu.writeRegister(0xFF14, 0x80); // NR14: trigger channel 1

    CHECK(apu.readRegister(0xFF26) == 0xF1); // channel 1 status bit set

    apu.writeRegister(0xFF26, 0x8F); // master power stays on; writing the read-only status bits has no effect
    CHECK(apu.readRegister(0xFF26) == 0xF1); // channel 1 status bit still set, unaffected by the write
}

TEST_CASE("Powering off via NR52 forcibly disables every channel")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF12, 0xF0); // NR12: volume 15 -> DAC enabled
    apu.writeRegister(0xFF14, 0x80); // NR14: trigger channel 1
    REQUIRE(apu.readRegister(0xFF26) == 0xF1);

    apu.writeRegister(0xFF26, 0x00); // power off
    CHECK(apu.readRegister(0xFF26) == 0x70); // master power and channel 1 status both cleared
}

TEST_CASE("Writing NR12/NR22/NR42 with the DAC-off pattern immediately disables an already-triggered channel")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF12, 0xF0); // DAC enabled
    apu.writeRegister(0xFF14, 0x80); // trigger channel 1
    CHECK((apu.getChannelEnabledMask() & 0x01) != 0);

    apu.writeRegister(0xFF12, 0x00); // volume 0, envelope decrease -> DAC off
    CHECK((apu.getChannelEnabledMask() & 0x01) == 0);
}

TEST_CASE("Triggering a channel with the DAC disabled leaves it disabled")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF21, 0x00); // NR42: volume 0, envelope decrease -> DAC off
    apu.writeRegister(0xFF23, 0x80); // trigger channel 4

    CHECK((apu.getChannelEnabledMask() & 0x08) == 0);
}

TEST_CASE("Wave RAM round-trips byte for byte")
{
    AudioProcessingUnit apu;
    apu.initialize();

    for(u16 address = 0xFF30; address <= 0xFF3F; ++address)
        apu.writeWaveRAM(address, static_cast<u8>(address - 0xFF30));

    for(u16 address = 0xFF30; address <= 0xFF3F; ++address)
        CHECK(apu.readWaveRAM(address) == static_cast<u8>(address - 0xFF30));
}

TEST_CASE("The unmapped 0xFF15 and 0xFF1F registers read 0xFF and ignore writes")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF15, 0x42);
    CHECK(apu.readRegister(0xFF15) == 0xFF);

    apu.writeRegister(0xFF1F, 0x42);
    CHECK(apu.readRegister(0xFF1F) == 0xFF);
}
