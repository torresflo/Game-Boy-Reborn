#pragma once

#include <vector>

#include "Common.h"
#include "AudioProcessingUnitTypes.h"
#include "save/ISaveStateSerializable.h"

class AudioProcessingUnit : public ISaveStateSerializable
{
public:
    static constexpr u32 ClockFrequencyHz = 4194304;
    static constexpr u32 FrameSequencerPeriodTicks = 8192; // ClockFrequencyHz / 512 Hz
    static constexpr u32 FrameSequencerStepCount = 8;
    static constexpr u32 OutputSampleRate = 44100;

    void initialize();
    void tick();

    //0xFF10-0xFF26
    u8 readRegister(u16 address) const;
    void writeRegister(u16 address, u8 value);

    //0xFF30-0xFF3F
    u8 readWaveRAM(u16 address) const;
    void writeWaveRAM(u16 address, u8 value);

    //Interleaved stereo samples (left0, right0, left1, right1, ...) generated since the last drain
    std::vector<s16> drainSampleBuffer();

    u8 getChannelEnabledMask() const; //NR52 bits 3-0

    virtual void serialize(SaveStateWriter& writer) const override;
    virtual void deserialize(SaveStateReader& reader) override;

private:
    static constexpr s32 TargetPeakAmplitude = 7000;

    //4 channels, each contributing 0-15, amplified by a master volume multiplier of 1-8
    static constexpr s32 MaxAmplifiedSample = 60 * 8;

    static void clockLengthCounter(u16& lengthTimer, bool lengthEnabled, bool& channelEnabled, u16 ceiling);
    static void clockEnvelope(u8 envelopeSweepPace, bool envelopeDirectionIncrease, u8& envelopeTimer, u8& currentVolume);

    void stepFrameSequencer();
    void clockLengthCounters();
    void clockEnvelopes();
    void clockSweep();
    
    void tickChannel1();
    void tickChannel2();
    void tickChannel3();
    void tickChannel4();
    
    u16 calculateSweepFrequency();
    void stepNoiseShiftRegister();

    void generateSampleIfDue();
    s16 mixSample(bool leftChannel) const;

    void powerOff();
    void powerOn();

    u8 readNR10() const;
    void writeNR10(u8 value);
    u8 readNR11() const;
    void writeNR11(u8 value);
    u8 readNR12() const;
    void writeNR12(u8 value);
    u8 readNR13() const;
    void writeNR13(u8 value);
    u8 readNR14() const;
    void writeNR14(u8 value);
    
    u8 readNR21() const;
    void writeNR21(u8 value);
    u8 readNR22() const;
    void writeNR22(u8 value);
    u8 readNR23() const;
    void writeNR23(u8 value);
    u8 readNR24() const;
    void writeNR24(u8 value);
    
    u8 readNR30() const;
    void writeNR30(u8 value);
    u8 readNR31() const;
    void writeNR31(u8 value);
    u8 readNR32() const;
    void writeNR32(u8 value);
    u8 readNR33() const;
    void writeNR33(u8 value);
    u8 readNR34() const;
    void writeNR34(u8 value);
    
    u8 readNR41() const;
    void writeNR41(u8 value);
    u8 readNR42() const;
    void writeNR42(u8 value);
    u8 readNR43() const;
    void writeNR43(u8 value);
    u8 readNR44() const;
    void writeNR44(u8 value);
    
    u8 readNR50() const;
    void writeNR50(u8 value);
    u8 readNR51() const;
    void writeNR51(u8 value);
    u8 readNR52() const;
    void writeNR52(u8 value);

    PulseSweepChannel channel1;
    PulseChannel channel2;
    WaveChannel channel3;
    NoiseChannel channel4;
    AudioControl control;

    u32 frameSequencerTickCounter = 0;
    u8 frameSequencerStep = 0;

    u32 sampleGenerationAccumulator = 0;
    std::vector<s16> sampleBuffer;

    friend class AudioProcessingUnitTestFixture;
};
