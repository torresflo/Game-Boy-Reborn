#include <doctest/doctest.h>

#include "AudioProcessingUnit.h"
#include "AudioProcessingUnitTestFixture.h"

namespace
{
    // stepFrameSequencer() acts on the CURRENT step value before advancing it, so the
    // transition that fires while frameSequencerStep == 2 is the 3rd transition overall
    // (the 1st and 2nd use step 0 and step 1), landing at tick 3 * FrameSequencerPeriodTicks.
    constexpr u32 TicksToFirstSweepClock = AudioProcessingUnit::FrameSequencerPeriodTicks * 3;
}

TEST_CASE("Sweep increases channel 1's frequency by shadowFrequency >> slope")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF12, 0xF0); // NR12: DAC enabled
    fixture.writeRegister(0xFF10, 0x11); // NR10: sweep pace 1, increase, slope 1
    fixture.writeRegister(0xFF13, 0x20); // NR13: period low (periodValue = 800)
    fixture.writeRegister(0xFF14, 0x83); // NR14: trigger + period high

    REQUIRE(fixture.channel1().periodValue == 800);

    for(u32 i = 0; i < TicksToFirstSweepClock; ++i)
        fixture.tick();

    // Chosen so neither the applied update nor the documented immediate re-check
    // overflows: 800 -> 1200 (applied) -> 1800 (re-check, discarded, no overflow).
    CHECK(fixture.channel1().periodValue == 1200); // 800 + (800 >> 1)
    CHECK(fixture.channel1().shadowFrequency == 1200);
    CHECK(fixture.getChannelEnabledMask() == 0x01);
}

TEST_CASE("Sweep decreases channel 1's frequency when the direction bit is set")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF12, 0xF0);
    fixture.writeRegister(0xFF10, 0x19); // NR10: pace 1, decrease, slope 1
    fixture.writeRegister(0xFF13, 0xE8); // periodValue low (1000)
    fixture.writeRegister(0xFF14, 0x83); // trigger + period high

    for(u32 i = 0; i < TicksToFirstSweepClock; ++i)
        fixture.tick();

    CHECK(fixture.channel1().periodValue == 500); // 1000 - (1000 >> 1)
}

TEST_CASE("Sweep overflow disables channel 1 without applying the new frequency")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF12, 0xF0);
    fixture.writeRegister(0xFF10, 0x11); // pace 1, increase, slope 1
    fixture.writeRegister(0xFF13, 0xD0); // periodValue low (2000)
    fixture.writeRegister(0xFF14, 0x87); // trigger + period high (2000 = 0x7D0)

    REQUIRE(fixture.channel1().periodValue == 2000);
    REQUIRE(fixture.getChannelEnabledMask() == 0x01);

    for(u32 i = 0; i < TicksToFirstSweepClock; ++i)
        fixture.tick();

    CHECK((fixture.getChannelEnabledMask() & 0x01) == 0); // 2000 + 1000 = 3000 > 2047, overflowed
    CHECK(fixture.channel1().periodValue == 2000); // not applied
}

TEST_CASE("Sweep's second overflow check can disable channel 1 even after a successful frequency update")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF12, 0xF0);
    fixture.writeRegister(0xFF10, 0x11); // pace 1, increase, slope 1
    fixture.writeRegister(0xFF13, 0x00); // periodValue low (1024)
    fixture.writeRegister(0xFF14, 0x84); // trigger + period high (1024 = 0x400)

    REQUIRE(fixture.channel1().periodValue == 1024);

    for(u32 i = 0; i < TicksToFirstSweepClock; ++i)
        fixture.tick();

    // First calculation: 1024 + (1024 >> 1) = 1536, within range -> applied.
    CHECK(fixture.channel1().periodValue == 1536);
    // Second calculation (re-checking overflow from the new shadow frequency):
    // 1536 + (1536 >> 1) = 2304 > 2047 -> disables the channel even though the
    // first update already succeeded (Pan Docs' documented double-check quirk).
    CHECK((fixture.getChannelEnabledMask() & 0x01) == 0);
}

TEST_CASE("Sweep with slope 0 never changes the frequency even though pace keeps the sweep timer running")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF12, 0xF0);
    fixture.writeRegister(0xFF10, 0x18); // pace 1, decrease, slope 0
    fixture.writeRegister(0xFF13, 0xE8); // periodValue low (1000)
    fixture.writeRegister(0xFF14, 0x83); // trigger + period high

    for(u32 i = 0; i < TicksToFirstSweepClock; ++i)
        fixture.tick();

    CHECK(fixture.channel1().periodValue == 1000); // slope 0 -> the calculated frequency is never applied
    CHECK(fixture.getChannelEnabledMask() == 0x01); // and the decrease-direction calculation didn't overflow either
}

TEST_CASE("Sweep pace 0 disables the sweep unit entirely")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF12, 0xF0);
    fixture.writeRegister(0xFF10, 0x01); // pace 0, increase, slope 1 -> sweepEnabled stays true (slope != 0) but pace gates clocking
    fixture.writeRegister(0xFF13, 0xE8); // periodValue low (1000)
    fixture.writeRegister(0xFF14, 0x83); // trigger + period high

    // sweepTimer reloads to 8 (since pace 0 reloads to the max) and ticks down across
    // multiple full frame-sequencer cycles; tick well past several reloads to confirm
    // the update is skipped every time, not just before the first reload.
    for(u32 i = 0; i < AudioProcessingUnit::FrameSequencerPeriodTicks * 8 * 5; ++i)
        fixture.tick();

    CHECK(fixture.channel1().periodValue == 1000); // never touched, regardless of how long we wait
    CHECK(fixture.getChannelEnabledMask() == 0x01);
}
