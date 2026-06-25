#include <doctest/doctest.h>

#include "AudioProcessingUnit.h"
#include "AudioProcessingUnitTestFixture.h"

TEST_CASE("Channel 4's LFSR produces the documented bit sequence from the 0x7FFF trigger state")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF21, 0xF0); // NR42: initial volume 15 -> DAC enabled
    fixture.writeRegister(0xFF22, 0x00); // NR43: clock shift 0, 15-bit mode, divisor code 0 -> 8 T-cycles/step
    fixture.writeRegister(0xFF23, 0x80); // NR44: trigger

    constexpr u32 ticksPerLFSRStep = 8;

    // Starting from 0x7FFF (15 bits set), bit 0 and bit 1 only first differ on the
    // 15th shift - until then the register keeps shifting in zeroes and bit 0 stays
    // set, so the channel stays silent.
    for(u32 step = 1; step <= 14; ++step)
    {
        for(u32 i = 0; i < ticksPerLFSRStep; ++i)
            fixture.tick();
        CHECK(fixture.channel4().getCurrentSample() == 0);
    }

    for(u32 i = 0; i < ticksPerLFSRStep; ++i)
        fixture.tick();
    CHECK(fixture.channel4().getCurrentSample() == 15); // 15th clock: bit 0 clears -> volume audible
    CHECK(fixture.channel4().noiseShiftRegister == 0x4000);
}

TEST_CASE("Channel 4's LFSR also folds the XOR result into bit 6 in 7-bit short mode")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF21, 0xF0); // DAC enabled
    fixture.writeRegister(0xFF22, 0x08); // NR43: short mode enabled, divisor 0, shift 0
    fixture.writeRegister(0xFF23, 0x80); // trigger

    // Start from a known, simple state where bit 0 and bit 1 differ, so the XOR
    // result for this single step is unambiguously true: bit0=1, bit1=0.
    fixture.setChannel4ShiftRegister(0x0001);

    constexpr u32 ticksPerLFSRStep = 8;
    for(u32 i = 0; i < ticksPerLFSRStep; ++i)
        fixture.tick();

    // (0x0001 >> 1) | (1 << 14) | (1 << 6) = 0x4040 - bit 14 (always) AND bit 6 (short mode only)
    CHECK(fixture.channel4().noiseShiftRegister == 0x4040);
}

TEST_CASE("Channel 4 with the DAC disabled reports silence and stays disabled when triggered")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF21, 0x00); // volume 0, envelope decrease -> DAC disabled
    fixture.writeRegister(0xFF23, 0x80); // trigger

    CHECK(fixture.channel4().getCurrentSample() == 0);
    CHECK((fixture.getChannelEnabledMask() & 0x08) == 0);
}
