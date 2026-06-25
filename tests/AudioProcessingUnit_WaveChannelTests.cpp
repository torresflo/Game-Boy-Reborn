#include <doctest/doctest.h>

#include "AudioProcessingUnit.h"
#include "AudioProcessingUnitTestFixture.h"

TEST_CASE("Wave channel plays back wave RAM nibbles high-then-low at the documented period")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF1A, 0x80); // NR30: DAC enabled
    fixture.writeRegister(0xFF1C, 0x20); // NR32: output level 1 -> 100%
    fixture.writeWaveRAM(0xFF30, 0x12);  // first byte: nibble0 = 1, nibble1 = 2
    fixture.writeWaveRAM(0xFF31, 0x34);  // second byte: nibble2 = 3, nibble3 = 4
    fixture.writeRegister(0xFF1D, 0xFE); // NR33: period low (periodValue = 2046)
    fixture.writeRegister(0xFF1E, 0x87); // NR34: trigger + period high

    constexpr u32 ticksPerSample = 4; // (2048 - 2046) * 2

    CHECK(fixture.channel3().getCurrentSample() == 1); // sample 0, right after trigger

    for(u32 i = 0; i < ticksPerSample; ++i)
        fixture.tick();
    CHECK(fixture.channel3().getCurrentSample() == 2); // sample 1

    for(u32 i = 0; i < ticksPerSample; ++i)
        fixture.tick();
    CHECK(fixture.channel3().getCurrentSample() == 3); // sample 2

    for(u32 i = 0; i < ticksPerSample; ++i)
        fixture.tick();
    CHECK(fixture.channel3().getCurrentSample() == 4); // sample 3
}

TEST_CASE("Wave channel output level shifts the nibble by 0, 1 or 2 bits")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF1A, 0x80); // NR30: DAC enabled
    fixture.writeWaveRAM(0xFF30, 0x80);  // nibble0 = 8 (1000b)
    fixture.writeRegister(0xFF1E, 0x80); // trigger

    fixture.writeRegister(0xFF1C, 0x20); // output level 1 -> 100%
    CHECK(fixture.channel3().getCurrentSample() == 8);

    fixture.writeRegister(0xFF1C, 0x40); // output level 2 -> 50%
    CHECK(fixture.channel3().getCurrentSample() == 4);

    fixture.writeRegister(0xFF1C, 0x60); // output level 3 -> 25%
    CHECK(fixture.channel3().getCurrentSample() == 2);

    fixture.writeRegister(0xFF1C, 0x00); // output level 0 -> mute
    CHECK(fixture.channel3().getCurrentSample() == 0);
}

TEST_CASE("Wave channel with the DAC disabled reports silence regardless of wave RAM content")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeWaveRAM(0xFF30, 0xFF);
    fixture.writeRegister(0xFF1C, 0x20); // output level 100%
    fixture.writeRegister(0xFF1E, 0x80); // trigger with the DAC disabled (NR30 never written)

    CHECK(fixture.channel3().getCurrentSample() == 0);
    CHECK((fixture.getChannelEnabledMask() & 0x04) == 0);
}
