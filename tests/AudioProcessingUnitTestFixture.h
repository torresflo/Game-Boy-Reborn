#pragma once

#include "AudioProcessingUnit.h"

// Test-only helper exposing AudioProcessingUnit's private channel/control state via
// friendship, mirroring CentralProcessingUnitJsonTestFixture's access pattern.
class AudioProcessingUnitTestFixture
{
public:
    AudioProcessingUnitTestFixture()
    {
        apu.initialize();
    }

    void tick() { apu.tick(); }
    void writeRegister(u16 address, u8 value) { apu.writeRegister(address, value); }
    u8 readRegister(u16 address) const { return apu.readRegister(address); }
    void writeWaveRAM(u16 address, u8 value) { apu.writeWaveRAM(address, value); }
    u8 readWaveRAM(u16 address) const { return apu.readWaveRAM(address); }
    std::vector<s16> drainSampleBuffer() { return apu.drainSampleBuffer(); }
    u8 getChannelEnabledMask() const { return apu.getChannelEnabledMask(); }
    s16 mixSample(bool leftChannel) const { return apu.mixSample(leftChannel); }

    const PulseSweepChannel& channel1() const { return apu.channel1; }
    const PulseChannel& channel2() const { return apu.channel2; }
    const WaveChannel& channel3() const { return apu.channel3; }
    const NoiseChannel& channel4() const { return apu.channel4; }
    const AudioControl& control() const { return apu.control; }

    void setChannel4ShiftRegister(u16 value) { apu.channel4.noiseShiftRegister = value; }

private:
    AudioProcessingUnit apu;
};
