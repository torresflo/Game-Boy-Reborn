#include <doctest/doctest.h>

#include "AudioProcessingUnit.h"
#include "AudioProcessingUnitTestFixture.h"

TEST_CASE("Mixer sums only channels routed to a given side via NR51")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF17, 0xF0); // NR22: volume 15 -> DAC enabled
    fixture.writeRegister(0xFF16, 0x80); // NR21: duty 50% (dutyPosition 0 -> "on" immediately after trigger)
    fixture.writeRegister(0xFF19, 0x80); // trigger channel 2

    fixture.writeRegister(0xFF24, 0x77); // NR50: volume 7 both sides
    fixture.writeRegister(0xFF25, 0x20); // NR51: channel 2 routed to LEFT only (bit 5)

    // sum=15, masterVolume=7: amplified = 15*8 = 120; centered = 120-240 = -120; scaled = -120*7000/240
    CHECK(fixture.mixSample(true) == -3500);
    // sum=0 (nothing routed to the right side): amplified=0; centered=-240; scaled=-240*7000/240
    CHECK(fixture.mixSample(false) == -7000);
}

TEST_CASE("Mixer's master volume scales the mix towards louder values as it increases")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF17, 0xF0);
    fixture.writeRegister(0xFF16, 0x80);
    fixture.writeRegister(0xFF19, 0x80);
    fixture.writeRegister(0xFF25, 0x20); // channel 2 -> left only

    fixture.writeRegister(0xFF24, 0x07); // NR50: left volume 0 (lowest)
    s16 quietestSample = fixture.mixSample(true);

    fixture.writeRegister(0xFF24, 0x77); // NR50: left volume 7 (highest)
    s16 loudestSample = fixture.mixSample(true);

    CHECK(loudestSample > quietestSample);
}

TEST_CASE("Master volume 0 still passes the signal through - it is not equivalent to silence")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF17, 0xF0);
    fixture.writeRegister(0xFF16, 0x80);
    fixture.writeRegister(0xFF19, 0x80);
    fixture.writeRegister(0xFF25, 0x20); // channel 2 -> left only
    fixture.writeRegister(0xFF24, 0x07); // NR50: left volume = 0 (documented as the lowest non-zero gain)

    s16 leftWithVolumeZero = fixture.mixSample(true);
    s16 rightWithNothingRouted = fixture.mixSample(false); // no channel routed to this side at all

    CHECK(leftWithVolumeZero != rightWithNothingRouted); // volume 0 still differs from "nothing routed"
}

TEST_CASE("Master power off produces exactly silent output regardless of channel or panning state")
{
    AudioProcessingUnitTestFixture fixture;

    fixture.writeRegister(0xFF17, 0xF0);
    fixture.writeRegister(0xFF16, 0x80);
    fixture.writeRegister(0xFF19, 0x80);
    fixture.writeRegister(0xFF25, 0xFF); // route every channel to both sides
    fixture.writeRegister(0xFF24, 0x77); // max master volume

    fixture.writeRegister(0xFF26, 0x00); // power off

    CHECK(fixture.mixSample(true) == 0);
    CHECK(fixture.mixSample(false) == 0);
}
