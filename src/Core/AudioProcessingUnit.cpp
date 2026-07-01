#include "AudioProcessingUnit.h"

#include <algorithm>
#include <format>
#include <utility>

#include "MathUtils.h"
#include "Save/SaveStateReader.h"
#include "Save/SaveStateWriter.h"

void AudioProcessingUnit::initialize()
{
    channel1.initialize();
    channel2.initialize();
    channel3.initialize();
    channel4.initialize();
    control.initialize();

    frameSequencerTickCounter = 0;
    frameSequencerStep = 0;

    sampleGenerationAccumulator = 0;
    sampleBuffer.clear();
}

void AudioProcessingUnit::tick()
{
    tickChannel1();
    tickChannel2();
    tickChannel3();
    tickChannel4();

    frameSequencerTickCounter++;
    if(frameSequencerTickCounter >= FrameSequencerPeriodTicks)
    {
        frameSequencerTickCounter = 0;
        stepFrameSequencer();
    }

    generateSampleIfDue();
}

void AudioProcessingUnit::clockLengthCounter(u16& lengthTimer, bool lengthEnabled, bool& channelEnabled, u16 ceiling)
{
    if(!lengthEnabled || lengthTimer >= ceiling)
        return;

    lengthTimer++;
    if(lengthTimer >= ceiling)
        channelEnabled = false;
}

void AudioProcessingUnit::clockEnvelope(u8 envelopeSweepPace, bool envelopeDirectionIncrease, u8& envelopeTimer, u8& currentVolume)
{
    if(envelopeSweepPace == 0)
        return;

    if(envelopeTimer > 0)
        envelopeTimer--;

    if(envelopeTimer == 0)
    {
        envelopeTimer = envelopeSweepPace;
        if(envelopeDirectionIncrease && currentVolume < 15)
            currentVolume++;
        else if(!envelopeDirectionIncrease && currentVolume > 0)
            currentVolume--;
    }
}

void AudioProcessingUnit::stepFrameSequencer()
{
    if(frameSequencerStep % 2 == 0)
        clockLengthCounters();
    if(frameSequencerStep == 2 || frameSequencerStep == 6)
        clockSweep();
    if(frameSequencerStep == 7)
        clockEnvelopes();

    frameSequencerStep = (frameSequencerStep + 1) % FrameSequencerStepCount;
}

void AudioProcessingUnit::clockLengthCounters()
{
    clockLengthCounter(channel1.lengthTimer, channel1.lengthEnabled, channel1.channelEnabled, 64);
    clockLengthCounter(channel2.lengthTimer, channel2.lengthEnabled, channel2.channelEnabled, 64);
    clockLengthCounter(channel3.lengthTimer, channel3.lengthEnabled, channel3.channelEnabled, 256);
    clockLengthCounter(channel4.lengthTimer, channel4.lengthEnabled, channel4.channelEnabled, 64);
}

void AudioProcessingUnit::clockEnvelopes()
{
    clockEnvelope(channel1.envelopeSweepPace, channel1.envelopeDirectionIncrease, channel1.envelopeTimer, channel1.currentVolume);
    clockEnvelope(channel2.envelopeSweepPace, channel2.envelopeDirectionIncrease, channel2.envelopeTimer, channel2.currentVolume);
    clockEnvelope(channel4.envelopeSweepPace, channel4.envelopeDirectionIncrease, channel4.envelopeTimer, channel4.currentVolume);
    //Channel 3 (wave) has no envelope
}

void AudioProcessingUnit::clockSweep()
{
    if(channel1.sweepTimer > 0)
        channel1.sweepTimer--;

    if(channel1.sweepTimer != 0)
        return;

    channel1.sweepTimer = (channel1.sweepPace == 0) ? 8 : channel1.sweepPace;

    if(!channel1.sweepEnabled || channel1.sweepPace == 0)
        return;

    u16 newFrequency = calculateSweepFrequency();
    if(newFrequency <= 2047 && channel1.sweepSlope > 0)
    {
        channel1.shadowFrequency = newFrequency;
        channel1.periodValue = newFrequency;
        calculateSweepFrequency(); //Second overflow check; result discarded
    }
}

void AudioProcessingUnit::tickChannel1()
{
    if(channel1.periodTimer > 0)
        channel1.periodTimer--;

    if(channel1.periodTimer == 0)
    {
        channel1.periodTimer = static_cast<u16>((2048 - channel1.periodValue) * 4);
        channel1.dutyPosition = (channel1.dutyPosition + 1) % 8;
    }
}

void AudioProcessingUnit::tickChannel2()
{
    if(channel2.periodTimer > 0)
        channel2.periodTimer--;

    if(channel2.periodTimer == 0)
    {
        channel2.periodTimer = static_cast<u16>((2048 - channel2.periodValue) * 4);
        channel2.dutyPosition = (channel2.dutyPosition + 1) % 8;
    }
}

void AudioProcessingUnit::tickChannel3()
{
    if(channel3.periodTimer > 0)
        channel3.periodTimer--;

    if(channel3.periodTimer == 0)
    {
        channel3.periodTimer = static_cast<u16>((2048 - channel3.periodValue) * 2);
        channel3.sampleIndex = (channel3.sampleIndex + 1) % 32;
    }
}

void AudioProcessingUnit::tickChannel4()
{
    if(channel4.periodTimer > 0)
        channel4.periodTimer--;

    if(channel4.periodTimer == 0)
    {
        channel4.periodTimer = channel4.getPeriodTimerReloadValue();
        stepNoiseShiftRegister();
    }
}

u16 AudioProcessingUnit::calculateSweepFrequency()
{
    u16 delta = channel1.shadowFrequency >> channel1.sweepSlope;
    u16 newFrequency = channel1.sweepDirectionDecrease ? (channel1.shadowFrequency - delta) : (channel1.shadowFrequency + delta);

    if(newFrequency > 2047)
        channel1.channelEnabled = false;

    return newFrequency;
}

void AudioProcessingUnit::stepNoiseShiftRegister()
{
    bool xorResult = MathUtils<u16>::getBitValue(channel4.noiseShiftRegister, 0) != MathUtils<u16>::getBitValue(channel4.noiseShiftRegister, 1);

    channel4.noiseShiftRegister >>= 1;
    MathUtils<u16>::setBitValue(channel4.noiseShiftRegister, 14, xorResult);

    if(channel4.shortModeEnabled)
        MathUtils<u16>::setBitValue(channel4.noiseShiftRegister, 6, xorResult);
}

void AudioProcessingUnit::generateSampleIfDue()
{
    sampleGenerationAccumulator += OutputSampleRate;
    if(sampleGenerationAccumulator >= ClockFrequencyHz)
    {
        sampleGenerationAccumulator -= ClockFrequencyHz;
        sampleBuffer.push_back(mixSample(true));
        sampleBuffer.push_back(mixSample(false));
    }
}

s16 AudioProcessingUnit::mixSample(bool leftChannel) const
{
    if(!control.masterEnabled)
        return 0;

    u32 sum = 0;
    if(leftChannel)
    {
        if(control.channel1Left) sum += channel1.getCurrentSample();
        if(control.channel2Left) sum += channel2.getCurrentSample();
        if(control.channel3Left) sum += channel3.getCurrentSample();
        if(control.channel4Left) sum += channel4.getCurrentSample();
    }
    else
    {
        if(control.channel1Right) sum += channel1.getCurrentSample();
        if(control.channel2Right) sum += channel2.getCurrentSample();
        if(control.channel3Right) sum += channel3.getCurrentSample();
        if(control.channel4Right) sum += channel4.getCurrentSample();
    }

    //Master volume 0-7 maps to a x1-x8 multiplier; volume 0 is the lowest non-zero
    //gain, not silence - only master power, DAC state, and panning produce silence.
    u8 masterVolume = leftChannel ? control.leftVolume : control.rightVolume;
    u32 amplified = sum * (masterVolume + 1); // 0 - MaxAmplifiedSample

    s32 centered = static_cast<s32>(amplified) - (MaxAmplifiedSample / 2);
    s32 scaled = centered * TargetPeakAmplitude / (MaxAmplifiedSample / 2);

    return static_cast<s16>(std::clamp(scaled, -32768, 32767));
}

void AudioProcessingUnit::powerOff()
{
    channel1.initialize();
    channel2.initialize();
    channel4.initialize();

    //Wave RAM is preserved across a power cycle; only channel 3's registers reset.
    std::array<u8, 16> preservedWaveRAM = channel3.waveRAM;
    channel3.initialize();
    channel3.waveRAM = preservedWaveRAM;

    control.vinLeftEnabled = false;
    control.leftVolume = 0;
    control.vinRightEnabled = false;
    control.rightVolume = 0;
    control.channel1Left = false; control.channel1Right = false;
    control.channel2Left = false; control.channel2Right = false;
    control.channel3Left = false; control.channel3Right = false;
    control.channel4Left = false; control.channel4Right = false;
}

void AudioProcessingUnit::powerOn()
{
    frameSequencerTickCounter = 0;
    frameSequencerStep = 0;
    channel1.dutyPosition = 0;
    channel2.dutyPosition = 0;
    channel3.sampleIndex = 0;
}

std::vector<s16> AudioProcessingUnit::drainSampleBuffer()
{
    std::vector<s16> drained;
    std::swap(drained, sampleBuffer);
    return drained;
}

u8 AudioProcessingUnit::getChannelEnabledMask() const
{
    return (channel4.channelEnabled ? 0x08 : 0)
         | (channel3.channelEnabled ? 0x04 : 0)
         | (channel2.channelEnabled ? 0x02 : 0)
         | (channel1.channelEnabled ? 0x01 : 0);
}

u8 AudioProcessingUnit::readRegister(u16 address) const
{
    switch(address)
    {
        case 0xFF10: return readNR10();
        case 0xFF11: return readNR11();
        case 0xFF12: return readNR12();
        case 0xFF13: return readNR13();
        case 0xFF14: return readNR14();
        case 0xFF15: return 0xFF; //Unmapped on real hardware
        case 0xFF16: return readNR21();
        case 0xFF17: return readNR22();
        case 0xFF18: return readNR23();
        case 0xFF19: return readNR24();
        case 0xFF1A: return readNR30();
        case 0xFF1B: return readNR31();
        case 0xFF1C: return readNR32();
        case 0xFF1D: return readNR33();
        case 0xFF1E: return readNR34();
        case 0xFF1F: return 0xFF; //Unmapped on real hardware
        case 0xFF20: return readNR41();
        case 0xFF21: return readNR42();
        case 0xFF22: return readNR43();
        case 0xFF23: return readNR44();
        case 0xFF24: return readNR50();
        case 0xFF25: return readNR51();
        case 0xFF26: return readNR52();
        default:
            Log::print(LogLevel::Error, std::format("Unsupported APU register reading (0x{:4X}).", address));
            return 0xFF;
    }
}

void AudioProcessingUnit::writeRegister(u16 address, u8 value)
{
    switch(address)
    {
        case 0xFF10:
            writeNR10(value); 
            break;
        case 0xFF11:
            writeNR11(value); 
            break;
        case 0xFF12:
            writeNR12(value); 
            break;
        case 0xFF13:
            writeNR13(value); 
            break;
        case 0xFF14:
            writeNR14(value); 
            break;
        case 0xFF15:
            break; //Unmapped on real hardware
        case 0xFF16:
            writeNR21(value); 
            break;
        case 0xFF17:
            writeNR22(value); 
            break;
        case 0xFF18:
            writeNR23(value); 
            break;
        case 0xFF19:
            writeNR24(value); 
            break;
        case 0xFF1A:
            writeNR30(value); 
            break;
        case 0xFF1B:
            writeNR31(value); 
            break;
        case 0xFF1C:
            writeNR32(value); 
            break;
        case 0xFF1D:
            writeNR33(value); 
            break;
        case 0xFF1E:
            writeNR34(value); 
            break;
        case 0xFF1F:
            break; //Unmapped on real hardware
        case 0xFF20:
            writeNR41(value); 
            break;
        case 0xFF21:
            writeNR42(value); 
            break;
        case 0xFF22:
            writeNR43(value); 
            break;
        case 0xFF23:
            writeNR44(value); 
            break;
        case 0xFF24:
            writeNR50(value); 
            break;
        case 0xFF25:
            writeNR51(value); 
            break;
        case 0xFF26:
            writeNR52(value); 
            break;
        default:
            Log::print(LogLevel::Error, std::format("Unsupported APU register writing (0x{:4X}).", address));
            break;
    }
}

u8 AudioProcessingUnit::readWaveRAM(u16 address) const
{
    return channel3.waveRAM[address - 0xFF30];
}

void AudioProcessingUnit::writeWaveRAM(u16 address, u8 value)
{
    channel3.waveRAM[address - 0xFF30] = value;
}

u8 AudioProcessingUnit::readNR10() const
{
    return static_cast<u8>(0x80 | (channel1.sweepPace << 4) | (channel1.sweepDirectionDecrease << 3) | channel1.sweepSlope);
}

void AudioProcessingUnit::writeNR10(u8 value)
{
    channel1.sweepPace = (value >> 4) & 0x7;
    channel1.sweepDirectionDecrease = MathUtils<u8>::getBitValue(value, 3);
    channel1.sweepSlope = value & 0x7;
}

u8 AudioProcessingUnit::readNR11() const
{
    return static_cast<u8>((channel1.waveDuty << 6) | 0x3F);
}

void AudioProcessingUnit::writeNR11(u8 value)
{
    channel1.waveDuty = (value >> 6) & 0x3;
    channel1.initialLengthTimer = value & 0x3F;
}

u8 AudioProcessingUnit::readNR12() const
{
    return static_cast<u8>((channel1.initialVolume << 4) | (channel1.envelopeDirectionIncrease << 3) | channel1.envelopeSweepPace);
}

void AudioProcessingUnit::writeNR12(u8 value)
{
    channel1.initialVolume = (value >> 4) & 0xF;
    channel1.envelopeDirectionIncrease = MathUtils<u8>::getBitValue(value, 3);
    channel1.envelopeSweepPace = value & 0x7;

    channel1.dacEnabled = channel1.initialVolume != 0 || channel1.envelopeDirectionIncrease;
    if(!channel1.dacEnabled)
        channel1.channelEnabled = false;
}

u8 AudioProcessingUnit::readNR13() const
{
    return 0xFF;
}

void AudioProcessingUnit::writeNR13(u8 value)
{
    channel1.periodValue = (channel1.periodValue & 0x700) | value;
}

u8 AudioProcessingUnit::readNR14() const
{
    return static_cast<u8>(0xBF | (channel1.lengthEnabled ? 0x40 : 0));
}

void AudioProcessingUnit::writeNR14(u8 value)
{
    channel1.lengthEnabled = MathUtils<u8>::getBitValue(value, 6);
    channel1.periodValue = (channel1.periodValue & 0xFF) | ((value & 0x7) << 8);

    if(MathUtils<u8>::getBitValue(value, 7))
        channel1.trigger();
}

u8 AudioProcessingUnit::readNR21() const
{
    return static_cast<u8>((channel2.waveDuty << 6) | 0x3F);
}

void AudioProcessingUnit::writeNR21(u8 value)
{
    channel2.waveDuty = (value >> 6) & 0x3;
    channel2.initialLengthTimer = value & 0x3F;
}

u8 AudioProcessingUnit::readNR22() const
{
    return static_cast<u8>((channel2.initialVolume << 4) | (channel2.envelopeDirectionIncrease << 3) | channel2.envelopeSweepPace);
}

void AudioProcessingUnit::writeNR22(u8 value)
{
    channel2.initialVolume = (value >> 4) & 0xF;
    channel2.envelopeDirectionIncrease = MathUtils<u8>::getBitValue(value, 3);
    channel2.envelopeSweepPace = value & 0x7;

    channel2.dacEnabled = channel2.initialVolume != 0 || channel2.envelopeDirectionIncrease;
    if(!channel2.dacEnabled)
        channel2.channelEnabled = false;
}

u8 AudioProcessingUnit::readNR23() const
{
    return 0xFF;
}

void AudioProcessingUnit::writeNR23(u8 value)
{
    channel2.periodValue = (channel2.periodValue & 0x700) | value;
}

u8 AudioProcessingUnit::readNR24() const
{
    return static_cast<u8>(0xBF | (channel2.lengthEnabled ? 0x40 : 0));
}

void AudioProcessingUnit::writeNR24(u8 value)
{
    channel2.lengthEnabled = MathUtils<u8>::getBitValue(value, 6);
    channel2.periodValue = (channel2.periodValue & 0xFF) | ((value & 0x7) << 8);

    if(MathUtils<u8>::getBitValue(value, 7))
        channel2.trigger();
}

u8 AudioProcessingUnit::readNR30() const
{
    return static_cast<u8>(0x7F | (channel3.dacEnabled ? 0x80 : 0));
}

void AudioProcessingUnit::writeNR30(u8 value)
{
    channel3.dacEnabled = MathUtils<u8>::getBitValue(value, 7);
    if(!channel3.dacEnabled)
        channel3.channelEnabled = false;
}

u8 AudioProcessingUnit::readNR31() const
{
    return 0xFF;
}

void AudioProcessingUnit::writeNR31(u8 value)
{
    channel3.initialLengthTimer = value;
}

u8 AudioProcessingUnit::readNR32() const
{
    return static_cast<u8>(0x9F | (channel3.outputLevel << 5));
}

void AudioProcessingUnit::writeNR32(u8 value)
{
    channel3.outputLevel = (value >> 5) & 0x3;
}

u8 AudioProcessingUnit::readNR33() const
{
    return 0xFF;
}

void AudioProcessingUnit::writeNR33(u8 value)
{
    channel3.periodValue = (channel3.periodValue & 0x700) | value;
}

u8 AudioProcessingUnit::readNR34() const
{
    return static_cast<u8>(0xBF | (channel3.lengthEnabled ? 0x40 : 0));
}

void AudioProcessingUnit::writeNR34(u8 value)
{
    channel3.lengthEnabled = MathUtils<u8>::getBitValue(value, 6);
    channel3.periodValue = (channel3.periodValue & 0xFF) | ((value & 0x7) << 8);

    if(MathUtils<u8>::getBitValue(value, 7))
        channel3.trigger();
}

u8 AudioProcessingUnit::readNR41() const
{
    return 0xFF;
}

void AudioProcessingUnit::writeNR41(u8 value)
{
    channel4.initialLengthTimer = value & 0x3F;
}

u8 AudioProcessingUnit::readNR42() const
{
    return static_cast<u8>((channel4.initialVolume << 4) | (channel4.envelopeDirectionIncrease << 3) | channel4.envelopeSweepPace);
}

void AudioProcessingUnit::writeNR42(u8 value)
{
    channel4.initialVolume = (value >> 4) & 0xF;
    channel4.envelopeDirectionIncrease = MathUtils<u8>::getBitValue(value, 3);
    channel4.envelopeSweepPace = value & 0x7;

    channel4.dacEnabled = channel4.initialVolume != 0 || channel4.envelopeDirectionIncrease;
    if(!channel4.dacEnabled)
        channel4.channelEnabled = false;
}

u8 AudioProcessingUnit::readNR43() const
{
    return static_cast<u8>((channel4.clockShift << 4) | (channel4.shortModeEnabled << 3) | channel4.clockDivider);
}

void AudioProcessingUnit::writeNR43(u8 value)
{
    channel4.clockShift = (value >> 4) & 0xF;
    channel4.shortModeEnabled = MathUtils<u8>::getBitValue(value, 3);
    channel4.clockDivider = value & 0x7;
}

u8 AudioProcessingUnit::readNR44() const
{
    return static_cast<u8>(0xBF | (channel4.lengthEnabled ? 0x40 : 0));
}

void AudioProcessingUnit::writeNR44(u8 value)
{
    channel4.lengthEnabled = MathUtils<u8>::getBitValue(value, 6);

    if(MathUtils<u8>::getBitValue(value, 7))
        channel4.trigger();
}

u8 AudioProcessingUnit::readNR50() const
{
    return static_cast<u8>((control.vinLeftEnabled << 7) | (control.leftVolume << 4) | (control.vinRightEnabled << 3) | control.rightVolume);
}

void AudioProcessingUnit::writeNR50(u8 value)
{
    control.vinLeftEnabled = MathUtils<u8>::getBitValue(value, 7);
    control.leftVolume = (value >> 4) & 0x7;
    control.vinRightEnabled = MathUtils<u8>::getBitValue(value, 3);
    control.rightVolume = value & 0x7;
}

u8 AudioProcessingUnit::readNR51() const
{
    return static_cast<u8>((control.channel4Left << 7) | (control.channel3Left << 6) | (control.channel2Left << 5) | (control.channel1Left << 4)
                          | (control.channel4Right << 3) | (control.channel3Right << 2) | (control.channel2Right << 1) | (control.channel1Right ? 1 : 0));
}

void AudioProcessingUnit::writeNR51(u8 value)
{
    control.channel4Left = MathUtils<u8>::getBitValue(value, 7);
    control.channel3Left = MathUtils<u8>::getBitValue(value, 6);
    control.channel2Left = MathUtils<u8>::getBitValue(value, 5);
    control.channel1Left = MathUtils<u8>::getBitValue(value, 4);
    control.channel4Right = MathUtils<u8>::getBitValue(value, 3);
    control.channel3Right = MathUtils<u8>::getBitValue(value, 2);
    control.channel2Right = MathUtils<u8>::getBitValue(value, 1);
    control.channel1Right = MathUtils<u8>::getBitValue(value, 0);
}

u8 AudioProcessingUnit::readNR52() const
{
    return static_cast<u8>(0x70 | (control.masterEnabled ? 0x80 : 0) | getChannelEnabledMask());
}

void AudioProcessingUnit::writeNR52(u8 value)
{
    bool newMasterEnabled = MathUtils<u8>::getBitValue(value, 7);

    if(control.masterEnabled && !newMasterEnabled)
    {
        control.masterEnabled = false;
        powerOff();
    }
    else if(!control.masterEnabled && newMasterEnabled)
    {
        control.masterEnabled = true;
        powerOn();
    }
}

const PulseSweepChannel& AudioProcessingUnit::getChannel1() const
{
    return channel1;
}

const PulseChannel& AudioProcessingUnit::getChannel2() const
{
    return channel2;
}

const WaveChannel& AudioProcessingUnit::getChannel3() const
{
    return channel3;
}

const NoiseChannel& AudioProcessingUnit::getChannel4() const
{
    return channel4;
}

const AudioControl& AudioProcessingUnit::getAudioControl() const
{
    return control;
}

void AudioProcessingUnit::serialize(SaveStateWriter& writer) const
{
    channel1.serialize(writer);
    channel2.serialize(writer);
    channel3.serialize(writer);
    channel4.serialize(writer);
    control.serialize(writer);

    writer.write(frameSequencerTickCounter);
    writer.write(frameSequencerStep);
    writer.write(sampleGenerationAccumulator);
}

void AudioProcessingUnit::deserialize(SaveStateReader& reader)
{
    channel1.deserialize(reader);
    channel2.deserialize(reader);
    channel3.deserialize(reader);
    channel4.deserialize(reader);
    control.deserialize(reader);

    reader.read(frameSequencerTickCounter);
    reader.read(frameSequencerStep);
    reader.read(sampleGenerationAccumulator);
}
