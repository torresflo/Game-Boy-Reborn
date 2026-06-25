#include "AudioProcessingUnitTypes.h"

#include "MathUtils.h"

namespace
{
    //Pulse duty waveforms (Pan Docs): 1 = output on, 0 = output off, indexed by dutyPosition (0-7)
    constexpr std::array<std::array<u8, 8>, 4> DutyWaveforms = {{
        {{0, 0, 0, 0, 0, 0, 0, 1}}, //12.5%
        {{1, 0, 0, 0, 0, 0, 0, 1}}, //25%
        {{1, 0, 0, 0, 0, 1, 1, 1}}, //50%
        {{0, 1, 1, 1, 1, 1, 1, 0}}, //75%
    }};

    //Noise channel period divisors (Pan Docs), indexed by the 3-bit clock divider code
    constexpr std::array<u16, 8> NoiseDivisors = {8, 16, 32, 48, 64, 80, 96, 112};
}

void PulseSweepChannel::initialize()
{
    sweepPace = 0;
    sweepDirectionDecrease = false;
    sweepSlope = 0;
    waveDuty = 0;
    initialLengthTimer = 0;
    initialVolume = 0;
    envelopeDirectionIncrease = false;
    envelopeSweepPace = 0;
    periodValue = 0;
    lengthEnabled = false;

    channelEnabled = false;
    dacEnabled = false;
    periodTimer = 0;
    dutyPosition = 0;
    lengthTimer = 0;
    currentVolume = 0;
    envelopeTimer = 0;
    shadowFrequency = 0;
    sweepTimer = 0;
    sweepEnabled = false;
}

void PulseSweepChannel::trigger()
{
    channelEnabled = dacEnabled;
    lengthTimer = initialLengthTimer;
    periodTimer = static_cast<u16>((2048 - periodValue) * 4);
    currentVolume = initialVolume;
    envelopeTimer = envelopeSweepPace;

    shadowFrequency = periodValue;
    sweepTimer = (sweepPace == 0) ? 8 : sweepPace;
    sweepEnabled = sweepPace != 0 || sweepSlope != 0;
}

u8 PulseSweepChannel::getCurrentSample() const
{
    if(!channelEnabled || !dacEnabled)
        return 0;

    return DutyWaveforms[waveDuty][dutyPosition] ? currentVolume : 0;
}

void PulseChannel::initialize()
{
    waveDuty = 0;
    initialLengthTimer = 0;
    initialVolume = 0;
    envelopeDirectionIncrease = false;
    envelopeSweepPace = 0;
    periodValue = 0;
    lengthEnabled = false;

    channelEnabled = false;
    dacEnabled = false;
    periodTimer = 0;
    dutyPosition = 0;
    lengthTimer = 0;
    currentVolume = 0;
    envelopeTimer = 0;
}

void PulseChannel::trigger()
{
    channelEnabled = dacEnabled;
    lengthTimer = initialLengthTimer;
    periodTimer = static_cast<u16>((2048 - periodValue) * 4);
    currentVolume = initialVolume;
    envelopeTimer = envelopeSweepPace;
}

u8 PulseChannel::getCurrentSample() const
{
    if(!channelEnabled || !dacEnabled)
        return 0;

    return DutyWaveforms[waveDuty][dutyPosition] ? currentVolume : 0;
}

void WaveChannel::initialize()
{
    dacEnabled = false;
    initialLengthTimer = 0;
    outputLevel = 0;
    periodValue = 0;
    lengthEnabled = false;
    waveRAM.fill(0);

    channelEnabled = false;
    periodTimer = 0;
    sampleIndex = 0;
    lengthTimer = 0;
}

void WaveChannel::trigger()
{
    channelEnabled = dacEnabled;
    lengthTimer = initialLengthTimer;
    periodTimer = static_cast<u16>((2048 - periodValue) * 2);
    sampleIndex = 0;
}

u8 WaveChannel::getCurrentSample() const
{
    if(!channelEnabled || !dacEnabled)
        return 0;

    u8 byte = waveRAM[sampleIndex / 2];
    u8 nibble = (sampleIndex % 2 == 0) ? static_cast<u8>(byte >> 4) : static_cast<u8>(byte & 0x0F);

    switch(outputLevel)
    {
        case 1: return nibble;             //100%
        case 2: return nibble >> 1;        //50%
        case 3: return nibble >> 2;        //25%
        default: return 0;                 //0: mute
    }
}

void NoiseChannel::initialize()
{
    initialLengthTimer = 0;
    initialVolume = 0;
    envelopeDirectionIncrease = false;
    envelopeSweepPace = 0;
    clockShift = 0;
    shortModeEnabled = false;
    clockDivider = 0;
    lengthEnabled = false;

    channelEnabled = false;
    dacEnabled = false;
    noiseShiftRegister = 0x7FFF;
    periodTimer = 0;
    lengthTimer = 0;
    currentVolume = 0;
    envelopeTimer = 0;
}

void NoiseChannel::trigger()
{
    channelEnabled = dacEnabled;
    lengthTimer = initialLengthTimer;
    currentVolume = initialVolume;
    envelopeTimer = envelopeSweepPace;
    noiseShiftRegister = 0x7FFF;
    periodTimer = getPeriodTimerReloadValue();
}

u8 NoiseChannel::getCurrentSample() const
{
    if(!channelEnabled || !dacEnabled)
        return 0;

    return MathUtils<u16>::getBitValue(noiseShiftRegister, 0) ? 0 : currentVolume;
}

u16 NoiseChannel::getPeriodTimerReloadValue() const
{
    return static_cast<u16>(NoiseDivisors[clockDivider] << clockShift);
}

void AudioControl::initialize()
{
    masterEnabled = true; //Real hardware powers the APU on by default at boot

    vinLeftEnabled = false;
    leftVolume = 0;
    vinRightEnabled = false;
    rightVolume = 0;

    channel1Left = false; channel1Right = false;
    channel2Left = false; channel2Right = false;
    channel3Left = false; channel3Right = false;
    channel4Left = false; channel4Right = false;
}
