#pragma once

#include <array>

#include <imgui.h>

#include "DebugPanel.h"
#include "AudioProcessingUnitTypes.h"

class ApuViewerPanel : public DebugPanel
{
public:
    ApuViewerPanel();

    void draw(GameBoyEmulator& emulator) override;

private:
    void drawSectionHeader(const char* label);
    void drawChannel1Section(const PulseSweepChannel& channel, const AudioControl& control);
    void drawChannel2Section(const PulseChannel& channel, const AudioControl& control);
    void drawChannel3Section(const WaveChannel& channel, const AudioControl& control);
    void drawChannel4Section(const NoiseChannel& channel, const AudioControl& control);
    void drawMasterControlSection(const AudioControl& control);

    void drawChannelStatusIndicator(bool channelEnabled, bool dacEnabled);
    void drawPanningIndicator(bool left, bool right);
    void drawEnvelopeState(u8 initialVolume, bool envelopeDirectionIncrease, u8 envelopeSweepPace, u8 currentVolume, u8 envelopeTimer);
    void drawDutyWaveform(u8 waveDuty, u8 dutyPosition);
    void drawLengthTimer(bool lengthEnabled, u16 lengthTimer);

    float calculatePulseFrequencyHz(u16 periodValue) const;
    float calculateWaveFrequencyHz(u16 periodValue) const;
    float calculateNoiseFrequencyHz(u8 clockShift, u8 clockDivider) const;

    std::array<float, 32> waveRamSamples{};

    static constexpr ImVec4 HeaderColor {0.6f,  0.6f,  0.65f, 1.f};
    static constexpr ImVec4 NameColor {0.55f, 0.75f, 1.f,   1.f};
    static constexpr ImVec4 ValueColor {1.f,   0.85f, 0.35f, 1.f};
    static constexpr ImVec4 EnabledColor {0.4f,  0.9f,  0.4f,  1.f};
    static constexpr ImVec4 DisabledColor {0.5f,  0.5f,  0.5f,  1.f};
    static constexpr ImVec4 DutyHighColor {1.f,   0.85f, 0.35f, 1.f};
    static constexpr ImVec4 DutyLowColor {0.35f, 0.35f, 0.35f, 1.f};
    static constexpr ImVec4 DutyCursorColor {1.f,   0.4f,  0.4f,  1.f};
};
