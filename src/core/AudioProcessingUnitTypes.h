#pragma once

#include <array>

#include "Common.h"
#include "save/ISaveStateSerializable.h"

// Channel 1: Pulse wave with frequency sweep (NR10-NR14, 0xFF10-0xFF14)
struct PulseSweepChannel : public ISaveStateSerializable
{
    //Raw register fields
    u8 sweepPace = 0;                      //NR10 bits 6-4
    bool sweepDirectionDecrease = false;   //NR10 bit 3 (false = increase, true = decrease)
    u8 sweepSlope = 0;                     //NR10 bits 2-0 ("individual step")
    u8 waveDuty = 0;                       //NR11 bits 7-6
    u8 initialLengthTimer = 0;             //NR11 bits 5-0 (write-only)
    u8 initialVolume = 0;                  //NR12 bits 7-4
    bool envelopeDirectionIncrease = false;//NR12 bit 3
    u8 envelopeSweepPace = 0;              //NR12 bits 2-0
    u16 periodValue = 0;                   //11 bits: NR13 (low 8) + NR14 bits 2-0 (high 3)
    bool lengthEnabled = false;            //NR14 bit 6

    //Runtime state
    bool channelEnabled = false;
    bool dacEnabled = false;
    u16 periodTimer = 0;
    u8 dutyPosition = 0;
    u16 lengthTimer = 0;
    u8 currentVolume = 0;
    u8 envelopeTimer = 0;
    u16 shadowFrequency = 0;
    u8 sweepTimer = 0;
    bool sweepEnabled = false;

    void initialize();
    void trigger();
    u8 getCurrentSample() const;

    virtual void serialize(SaveStateWriter& writer) const override;
    virtual void deserialize(SaveStateReader& reader) override;
};

// Channel 2: Pulse wave without sweep (NR21-NR24, 0xFF16-0xFF19)
struct PulseChannel : public ISaveStateSerializable
{
    //Raw register fields
    u8 waveDuty = 0;                       //NR21 bits 7-6
    u8 initialLengthTimer = 0;             //NR21 bits 5-0 (write-only)
    u8 initialVolume = 0;                  //NR22 bits 7-4
    bool envelopeDirectionIncrease = false;//NR22 bit 3
    u8 envelopeSweepPace = 0;              //NR22 bits 2-0
    u16 periodValue = 0;                   //11 bits: NR23 (low 8) + NR24 bits 2-0 (high 3)
    bool lengthEnabled = false;            //NR24 bit 6

    //Runtime state
    bool channelEnabled = false;
    bool dacEnabled = false;
    u16 periodTimer = 0;
    u8 dutyPosition = 0;
    u16 lengthTimer = 0;
    u8 currentVolume = 0;
    u8 envelopeTimer = 0;

    void initialize();
    void trigger();
    u8 getCurrentSample() const;

    virtual void serialize(SaveStateWriter& writer) const override;
    virtual void deserialize(SaveStateReader& reader) override;
};

// Channel 3: Wave (NR30-NR34 + wave RAM, 0xFF1A-0xFF1E / 0xFF30-0xFF3F)
struct WaveChannel : public ISaveStateSerializable
{
    //Raw register fields
    bool dacEnabled = false;       //NR30 bit 7
    u8 initialLengthTimer = 0;     //NR31, full 8 bits (write-only)
    u8 outputLevel = 0;            //NR32 bits 6-5 (0=mute,1=100%,2=50%,3=25%)
    u16 periodValue = 0;           //11 bits: NR33 (low 8) + NR34 bits 2-0 (high 3)
    bool lengthEnabled = false;    //NR34 bit 6
    std::array<u8, 16> waveRAM{};  //0xFF30-0xFF3F, 32 packed 4-bit samples (high nibble first)

    //Runtime state
    bool channelEnabled = false;
    u16 periodTimer = 0;
    u8 sampleIndex = 0;
    u16 lengthTimer = 0;

    void initialize();
    void trigger();
    u8 getCurrentSample() const;

    virtual void serialize(SaveStateWriter& writer) const override;
    virtual void deserialize(SaveStateReader& reader) override;
};

// Channel 4: Noise (NR41-NR44, 0xFF20-0xFF23)
struct NoiseChannel : public ISaveStateSerializable
{
    //Raw register fields
    u8 initialLengthTimer = 0;             //NR41 bits 5-0 (write-only)
    u8 initialVolume = 0;                  //NR42 bits 7-4
    bool envelopeDirectionIncrease = false;//NR42 bit 3
    u8 envelopeSweepPace = 0;              //NR42 bits 2-0
    u8 clockShift = 0;                     //NR43 bits 7-4
    bool shortModeEnabled = false;         //NR43 bit 3 (false = 15-bit LFSR, true = 7-bit)
    u8 clockDivider = 0;                   //NR43 bits 2-0
    bool lengthEnabled = false;            //NR44 bit 6

    //Runtime state
    bool channelEnabled = false;
    bool dacEnabled = false;
    u16 noiseShiftRegister = 0x7FFF;
    u16 periodTimer = 0;
    u16 lengthTimer = 0;
    u8 currentVolume = 0;
    u8 envelopeTimer = 0;

    void initialize();
    void trigger();
    u8 getCurrentSample() const;
    u16 getPeriodTimerReloadValue() const;

    virtual void serialize(SaveStateWriter& writer) const override;
    virtual void deserialize(SaveStateReader& reader) override;
};

// Mixer / control (NR50-NR52, 0xFF24-0xFF26)
struct AudioControl : public ISaveStateSerializable
{
    bool masterEnabled = false;    //NR52 bit 7

    bool vinLeftEnabled = false;   //NR50 bit 7 (cartridge VIN input, unused on DMG in practice)
    u8 leftVolume = 0;             //NR50 bits 6-4 (0-7)
    bool vinRightEnabled = false;  //NR50 bit 3
    u8 rightVolume = 0;            //NR50 bits 2-0 (0-7)

    //NR51 panning bits - true means that channel's output is routed to that side
    bool channel1Left = false, channel1Right = false;
    bool channel2Left = false, channel2Right = false;
    bool channel3Left = false, channel3Right = false;
    bool channel4Left = false, channel4Right = false;

    void initialize();

    virtual void serialize(SaveStateWriter& writer) const override;
    virtual void deserialize(SaveStateReader& reader) override;
};
