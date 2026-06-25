#include <doctest/doctest.h>

#include "AudioProcessingUnit.h"
#include "AudioProcessingUnitTestFixture.h"

TEST_CASE("Channel 2 plays audibly after trigger, decays via envelope, and silences when its length expires")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF17, 0x82); // NR22: initial volume 8, envelope decrease, pace 2
    fixture.writeRegister(0xFF16, 0x80); // NR21: duty 50%, length = 0 (max duration: 64 clocks until expiry)
    fixture.writeRegister(0xFF18, 0xF8); // NR23: period low
    fixture.writeRegister(0xFF19, 0xC7); // NR24: trigger + length enable + period high (periodValue = 2040)

    REQUIRE(fixture.getChannelEnabledMask() == 0x02);
    REQUIRE(fixture.channel2().getCurrentSample() == 8); // audible immediately after trigger

    // Envelope pace 2: the volume only steps down on the 2nd step-7 clock, i.e. after
    // 2 full 8-step frame sequencer cycles.
    constexpr u32 ticksToFirstEnvelopeDecay = AudioProcessingUnit::FrameSequencerPeriodTicks * 8 * 2;
    for(u32 i = 0; i < ticksToFirstEnvelopeDecay; ++i)
        fixture.tick();
    CHECK(fixture.channel2().currentVolume == 7); // decayed by one step
    CHECK(fixture.getChannelEnabledMask() == 0x02); // still playing - length hasn't expired yet

    // A length of 0 needs the full 64 clocks (at 256 Hz) to expire; run the
    // remaining time out from where the envelope check left off.
    constexpr u32 ticksToLengthExpiry = AudioProcessingUnit::FrameSequencerPeriodTicks
                                       + 63 * AudioProcessingUnit::FrameSequencerPeriodTicks * 2;
    for(u32 i = ticksToFirstEnvelopeDecay; i < ticksToLengthExpiry; ++i)
        fixture.tick();

    CHECK((fixture.getChannelEnabledMask() & 0x02) == 0); // length expired, channel disabled
    CHECK(fixture.channel2().getCurrentSample() == 0);    // silent once disabled
}

TEST_CASE("Retriggering channel 1 after a sweep overflow disable brings it back to life")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF12, 0xF0); // NR12: DAC enabled
    fixture.writeRegister(0xFF11, 0x80); // NR11: duty 50% (dutyPosition 0 is "on")
    fixture.writeRegister(0xFF10, 0x11); // NR10: sweep pace 1, increase, slope 1
    fixture.writeRegister(0xFF13, 0xD0); // NR13: period low (periodValue = 2000)
    fixture.writeRegister(0xFF14, 0x87); // NR14: trigger + period high

    constexpr u32 ticksToFirstSweepClock = AudioProcessingUnit::FrameSequencerPeriodTicks * 3;
    for(u32 i = 0; i < ticksToFirstSweepClock; ++i)
        fixture.tick();
    REQUIRE((fixture.getChannelEnabledMask() & 0x01) == 0); // overflowed and disabled

    fixture.writeRegister(0xFF13, 0x00); // lower the period back into range before retriggering
    fixture.writeRegister(0xFF14, 0x84); // retrigger
    CHECK(fixture.getChannelEnabledMask() == 0x01);
    CHECK(fixture.channel1().getCurrentSample() > 0);
}
