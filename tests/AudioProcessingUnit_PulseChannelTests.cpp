#include <array>

#include <doctest/doctest.h>

#include "AudioProcessingUnit.h"
#include "AudioProcessingUnitTestFixture.h"

TEST_CASE("Channel 2 produces the documented 50% duty waveform pattern at the right period")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF17, 0xF0); // NR22: initial volume 15 -> DAC enabled
    fixture.writeRegister(0xFF16, 0x80); // NR21: duty 50%
    fixture.writeRegister(0xFF18, 0xF8); // NR23: period low
    fixture.writeRegister(0xFF19, 0x87); // NR24: trigger + period high (periodValue = 0x7F8 = 2040 -> 32 T-cycles/duty step)

    constexpr u32 ticksPerDutyStep = 32; // (2048 - 2040) * 4
    constexpr std::array<bool, 8> expectedOn = {true, false, false, false, false, true, true, true}; // 50% duty

    for(bool on : expectedOn)
    {
        fixture.tick();
        CHECK((fixture.channel2().getCurrentSample() > 0) == on);

        for(u32 i = 1; i < ticksPerDutyStep; ++i)
            fixture.tick();
    }
}

TEST_CASE("Channel 1 produces duty waveform output the same way channel 2 does")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF12, 0xF0); // NR12: initial volume 15 -> DAC enabled
    fixture.writeRegister(0xFF11, 0x40); // NR11: duty 25%
    fixture.writeRegister(0xFF13, 0xF8); // NR13: period low
    fixture.writeRegister(0xFF14, 0x87); // NR14: trigger + period high (periodValue = 2040 -> 32 T-cycles/duty step)

    constexpr u32 ticksPerDutyStep = 32;
    constexpr std::array<bool, 8> expectedOn = {true, false, false, false, false, false, false, true}; // 25% duty

    for(bool on : expectedOn)
    {
        fixture.tick();
        CHECK((fixture.channel1().getCurrentSample() > 0) == on);

        for(u32 i = 1; i < ticksPerDutyStep; ++i)
            fixture.tick();
    }
}

TEST_CASE("Channel 2 with the DAC disabled always reports a silent sample regardless of duty/volume")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF17, 0x00); // NR22: volume 0, envelope decrease -> DAC disabled
    fixture.writeRegister(0xFF16, 0x80); // NR21: duty 50%
    fixture.writeRegister(0xFF19, 0x80); // trigger (channel stays disabled since DAC is off)

    CHECK(fixture.channel2().getCurrentSample() == 0);
}

TEST_CASE("Triggering channel 2 reloads the length counter from the just-written NR21 value")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF17, 0xF0); // DAC enabled
    fixture.writeRegister(0xFF16, 0x3E); // NR21: length = 62 -> only 2 clocks until expiry (64 - 62)
    fixture.writeRegister(0xFF19, 0xC0); // trigger + length enable

    REQUIRE(fixture.getChannelEnabledMask() == 0x02);

    constexpr u32 ticksToFirstLengthClock = AudioProcessingUnit::FrameSequencerPeriodTicks;
    constexpr u32 ticksBetweenLengthClocks = AudioProcessingUnit::FrameSequencerPeriodTicks * 2;

    for(u32 i = 0; i < ticksToFirstLengthClock; ++i)
        fixture.tick();
    CHECK(fixture.getChannelEnabledMask() == 0x02); // 1st clock: 62 -> 63, still enabled

    for(u32 i = 0; i < ticksBetweenLengthClocks; ++i)
        fixture.tick();
    CHECK((fixture.getChannelEnabledMask() & 0x02) == 0); // 2nd clock: 63 -> 64, disabled
}

TEST_CASE("Retriggering channel 2 after it was length-disabled re-enables it")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF17, 0xF0); // DAC enabled
    fixture.writeRegister(0xFF16, 0x3F); // NR21: length = 63 -> 1 clock until expiry
    fixture.writeRegister(0xFF19, 0xC0); // trigger + length enable

    constexpr u32 ticksToFirstLengthClock = AudioProcessingUnit::FrameSequencerPeriodTicks;
    for(u32 i = 0; i < ticksToFirstLengthClock; ++i)
        fixture.tick();
    REQUIRE((fixture.getChannelEnabledMask() & 0x02) == 0); // expired

    fixture.writeRegister(0xFF19, 0xC0); // retrigger
    CHECK(fixture.getChannelEnabledMask() == 0x02);
}
