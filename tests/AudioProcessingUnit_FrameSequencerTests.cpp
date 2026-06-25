#include <doctest/doctest.h>

#include "AudioProcessingUnit.h"
#include "AudioProcessingUnitTestFixture.h"

TEST_CASE("Frame sequencer clocks channel 1's length counter at 256 Hz and disables it after 64 clocks")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF12, 0xF0); // NR12: initial volume 15 -> DAC enabled
    apu.writeRegister(0xFF14, 0xC0); // NR14: trigger (bit 7) + length enable (bit 6)

    REQUIRE(apu.getChannelEnabledMask() == 0x01);

    // Length is clocked on frame sequencer steps 0, 2, 4 and 6: the first clock lands
    // at tick 8192 (end of step 0), and every clock after that is 16384 ticks apart.
    constexpr u32 ticksToFirstLengthClock = AudioProcessingUnit::FrameSequencerPeriodTicks;
    constexpr u32 ticksBetweenLengthClocks = AudioProcessingUnit::FrameSequencerPeriodTicks * 2;
    constexpr u32 ticksFor64Clocks = ticksToFirstLengthClock + 63 * ticksBetweenLengthClocks;

    for(u32 i = 0; i < ticksFor64Clocks - 1; ++i)
        apu.tick();
    CHECK(apu.getChannelEnabledMask() == 0x01); // the 63rd clock just landed, still enabled

    apu.tick();
    CHECK((apu.getChannelEnabledMask() & 0x01) == 0); // the 64th clock disables the channel
}

TEST_CASE("Frame sequencer clocks channel 3's length counter to a 256 ceiling")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF1A, 0x80); // NR30: DAC enabled
    apu.writeRegister(0xFF1E, 0xC0); // NR34: trigger + length enable

    REQUIRE(apu.getChannelEnabledMask() == 0x04);

    constexpr u32 ticksToFirstLengthClock = AudioProcessingUnit::FrameSequencerPeriodTicks;
    constexpr u32 ticksBetweenLengthClocks = AudioProcessingUnit::FrameSequencerPeriodTicks * 2;
    constexpr u32 ticksFor256Clocks = ticksToFirstLengthClock + 255 * ticksBetweenLengthClocks;

    for(u32 i = 0; i < ticksFor256Clocks - 1; ++i)
        apu.tick();
    CHECK(apu.getChannelEnabledMask() == 0x04);

    apu.tick();
    CHECK((apu.getChannelEnabledMask() & 0x04) == 0);
}

TEST_CASE("Length counter does not disable a channel when length is not enabled")
{
    AudioProcessingUnit apu;
    apu.initialize();

    apu.writeRegister(0xFF12, 0xF0); // DAC enabled
    apu.writeRegister(0xFF14, 0x80); // trigger, length NOT enabled

    REQUIRE(apu.getChannelEnabledMask() == 0x01);

    constexpr u32 ticksFor100Clocks = AudioProcessingUnit::FrameSequencerPeriodTicks
                                     + 99 * AudioProcessingUnit::FrameSequencerPeriodTicks * 2;
    for(u32 i = 0; i < ticksFor100Clocks; ++i)
        apu.tick();

    CHECK(apu.getChannelEnabledMask() == 0x01); // never disabled since length is not enabled
}

TEST_CASE("Envelope clocks once per full frame sequencer cycle (on step 7) and increments volume")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF12, 0x09); // NR12: initial volume 0, envelope direction increase, pace 1
    fixture.writeRegister(0xFF14, 0x80); // trigger channel 1

    REQUIRE(fixture.channel1().currentVolume == 0);

    // Envelope is clocked once per full 8-step frame sequencer cycle (on step 7),
    // i.e. every 8 * FrameSequencerPeriodTicks T-cycles.
    constexpr u32 ticksPerEnvelopeClock = AudioProcessingUnit::FrameSequencerPeriodTicks * 8;

    for(u32 i = 0; i < ticksPerEnvelopeClock - 1; ++i)
        fixture.tick();
    CHECK(fixture.channel1().currentVolume == 0); // envelope clock hasn't landed yet

    fixture.tick();
    CHECK(fixture.channel1().currentVolume == 1); // envelope clock landed, volume incremented

    // Repeated clocks must clamp at 15, never wrap past it.
    for(u32 clock = 0; clock < 20; ++clock)
        for(u32 i = 0; i < ticksPerEnvelopeClock; ++i)
            fixture.tick();
    CHECK(fixture.channel1().currentVolume == 15);
}

TEST_CASE("Sample generation accumulator produces exactly OutputSampleRate stereo pairs per second of T-cycles")
{
    AudioProcessingUnit apu;
    apu.initialize();

    for(u32 i = 0; i < AudioProcessingUnit::ClockFrequencyHz; ++i)
        apu.tick();

    std::vector<s16> samples = apu.drainSampleBuffer();
    CHECK(samples.size() == AudioProcessingUnit::OutputSampleRate * 2); // stereo, interleaved

    CHECK(apu.drainSampleBuffer().empty()); // buffer is cleared after draining
}
