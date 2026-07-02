#include "ApuViewerPanel.h"

#include <array>
#include <format>

#include <imgui.h>

#include "AudioProcessingUnit.h"
#include "GameBoyEmulator.h"

ApuViewerPanel::ApuViewerPanel()
    : DebugPanel("APU Viewer")
{
}

void ApuViewerPanel::draw(GameBoyEmulator& emulator)
{
    const AudioProcessingUnit& apu = emulator.getAPU();
    const AudioControl& control = apu.getAudioControl();

    drawChannel1Section(apu.getChannel1(), control);
    drawChannel2Section(apu.getChannel2(), control);
    drawChannel3Section(apu.getChannel3(), control);
    drawChannel4Section(apu.getChannel4(), control);
    drawMasterControlSection(control);
}

void ApuViewerPanel::drawSectionHeader(const char* label)
{
    ImGui::Spacing();
    ImGui::TextColored(HeaderColor, "%s", label);
    ImGui::Separator();
}

void ApuViewerPanel::drawChannelStatusIndicator(bool channelEnabled, bool dacEnabled)
{
    ImGui::TextColored(channelEnabled ? EnabledColor : DisabledColor,
                       channelEnabled ? "[ON] " : "[OFF]");
    ImGui::SameLine();
    ImGui::TextColored(dacEnabled ? EnabledColor : DisabledColor,
                       dacEnabled ? "DAC: ON" : "DAC: OFF");
}

void ApuViewerPanel::drawPanningIndicator(bool left, bool right)
{
    ImGui::TextColored(NameColor, "Pan:");
    ImGui::SameLine();
    ImGui::TextColored(left  ? EnabledColor : DisabledColor, "L");
    ImGui::SameLine();
    ImGui::TextColored(right ? EnabledColor : DisabledColor, "R");
}

void ApuViewerPanel::drawEnvelopeState(u8 initialVolume, bool envelopeDirectionIncrease,
                                        u8 envelopeSweepPace, u8 currentVolume, u8 envelopeTimer)
{
    ImGui::TextColored(NameColor, "Volume:");
    ImGui::SameLine();
    ImGui::TextColored(ValueColor, "%u / 15  (init: %u  %s  pace: %u  timer: %u)",
        currentVolume, initialVolume,
        envelopeDirectionIncrease ? "+" : "-",
        envelopeSweepPace,
        envelopeTimer);
}

void ApuViewerPanel::drawDutyWaveform(u8 waveDuty, u8 dutyPosition)
{
    static constexpr std::array<std::array<u8, 8>, 4> DutyPatterns = {{
        {{0, 0, 0, 0, 0, 0, 0, 1}},
        {{1, 0, 0, 0, 0, 0, 0, 1}},
        {{1, 0, 0, 0, 0, 1, 1, 1}},
        {{0, 1, 1, 1, 1, 1, 1, 0}},
    }};
    static constexpr std::array<const char*, 4> DutyLabels = {"12.5%", "25%", "50%", "75%"};

    ImGui::TextColored(NameColor, "Duty (%s):", DutyLabels[waveDuty]);
    ImGui::SameLine();

    const auto& pattern = DutyPatterns[waveDuty];
    for(u8 step = 0; step < 8; ++step)
    {
        if(step > 0)
            ImGui::SameLine(0.f, 2.f);

        bool isHigh = (pattern[step] != 0);
        bool isCurrent = (step == dutyPosition);
        ImVec4 color = isCurrent ? DutyCursorColor : (isHigh ? DutyHighColor : DutyLowColor);
        ImGui::TextColored(color, isHigh ? "^" : "_");
    }
}

void ApuViewerPanel::drawLengthTimer(bool lengthEnabled, u16 lengthTimer)
{
    ImGui::TextColored(NameColor, "Length:");
    ImGui::SameLine();
    if(lengthEnabled)
        ImGui::TextColored(ValueColor, "%u (enabled)", lengthTimer);
    else
        ImGui::TextColored(DisabledColor, "%u (disabled)", lengthTimer);
}

float ApuViewerPanel::calculatePulseFrequencyHz(u16 periodValue) const
{
    if(periodValue >= 2048)
        return 0.f;
    return 131072.f / static_cast<float>(2048 - periodValue);
}

float ApuViewerPanel::calculateWaveFrequencyHz(u16 periodValue) const
{
    if(periodValue >= 2048)
        return 0.f;
    return 65536.f / static_cast<float>(2048 - periodValue);
}

float ApuViewerPanel::calculateNoiseFrequencyHz(u8 clockShift, u8 clockDivider) const
{
    static constexpr std::array<u16, 8> NoiseDivisors = {8, 16, 32, 48, 64, 80, 96, 112};
    u32 period = static_cast<u32>(NoiseDivisors[clockDivider]) << clockShift;
    if(period == 0)
        return 0.f;
    return 4194304.f / static_cast<float>(period);
}

void ApuViewerPanel::drawChannel1Section(const PulseSweepChannel& channel, const AudioControl& control)
{
    drawSectionHeader("Channel 1 - Pulse + Sweep (NR10-NR14)");
    drawChannelStatusIndicator(channel.channelEnabled, channel.dacEnabled);
    ImGui::SameLine(0.f, 16.f);
    drawPanningIndicator(control.channel1Left, control.channel1Right);

    drawDutyWaveform(channel.waveDuty, channel.dutyPosition);
    drawEnvelopeState(channel.initialVolume, channel.envelopeDirectionIncrease,
                      channel.envelopeSweepPace, channel.currentVolume, channel.envelopeTimer);
    drawLengthTimer(channel.lengthEnabled, channel.lengthTimer);

    ImGui::TextColored(NameColor, "Frequency:");
    ImGui::SameLine();
    ImGui::TextColored(ValueColor, "%.1f Hz  (period: %u)",
        calculatePulseFrequencyHz(channel.periodValue), channel.periodValue);

    ImGui::TextColored(NameColor, "Sweep:");
    ImGui::SameLine();
    ImGui::TextColored(ValueColor,
        "pace: %u  slope: %u  %s  enabled: %s  shadow: %u  timer: %u",
        channel.sweepPace, channel.sweepSlope,
        channel.sweepDirectionDecrease ? "decrease" : "increase",
        channel.sweepEnabled ? "yes" : "no",
        channel.shadowFrequency, channel.sweepTimer);
}

void ApuViewerPanel::drawChannel2Section(const PulseChannel& channel, const AudioControl& control)
{
    drawSectionHeader("Channel 2 - Pulse (NR21-NR24)");
    drawChannelStatusIndicator(channel.channelEnabled, channel.dacEnabled);
    ImGui::SameLine(0.f, 16.f);
    drawPanningIndicator(control.channel2Left, control.channel2Right);

    drawDutyWaveform(channel.waveDuty, channel.dutyPosition);
    drawEnvelopeState(channel.initialVolume, channel.envelopeDirectionIncrease,
                      channel.envelopeSweepPace, channel.currentVolume, channel.envelopeTimer);
    drawLengthTimer(channel.lengthEnabled, channel.lengthTimer);

    ImGui::TextColored(NameColor, "Frequency:");
    ImGui::SameLine();
    ImGui::TextColored(ValueColor, "%.1f Hz  (period: %u)",
        calculatePulseFrequencyHz(channel.periodValue), channel.periodValue);
}

void ApuViewerPanel::drawChannel3Section(const WaveChannel& channel, const AudioControl& control)
{
    drawSectionHeader("Channel 3 - Wave (NR30-NR34 + wave RAM)");
    drawChannelStatusIndicator(channel.channelEnabled, channel.dacEnabled);
    ImGui::SameLine(0.f, 16.f);
    drawPanningIndicator(control.channel3Left, control.channel3Right);

    static constexpr std::array<const char*, 4> OutputLevelNames = {"mute", "100%", "50%", "25%"};
    u8 safeLevel = channel.outputLevel < 4 ? channel.outputLevel : 0;
    ImGui::TextColored(NameColor, "Output level:");
    ImGui::SameLine();
    ImGui::TextColored(ValueColor, "%s", OutputLevelNames[safeLevel]);

    drawLengthTimer(channel.lengthEnabled, channel.lengthTimer);

    ImGui::TextColored(NameColor, "Frequency:");
    ImGui::SameLine();
    ImGui::TextColored(ValueColor, "%.1f Hz  (period: %u)",
        calculateWaveFrequencyHz(channel.periodValue), channel.periodValue);

    ImGui::TextColored(NameColor, "Sample index:");
    ImGui::SameLine();
    ImGui::TextColored(ValueColor, "%u / 31", channel.sampleIndex);

    for(u8 byteIndex = 0; byteIndex < 16; ++byteIndex)
    {
        waveRamSamples[byteIndex * 2]     = static_cast<float>((channel.waveRAM[byteIndex] >> 4) & 0xF);
        waveRamSamples[byteIndex * 2 + 1] = static_cast<float>(channel.waveRAM[byteIndex] & 0xF);
    }

    ImGui::TextColored(NameColor, "Wave RAM:");
    ImGui::PlotLines("##waveRAM", waveRamSamples.data(),
                     static_cast<int>(waveRamSamples.size()),
                     static_cast<int>(channel.sampleIndex),
                     nullptr, 0.f, 15.f,
                     ImVec2(ImGui::GetContentRegionAvail().x - 20.f, 40.f));
}

void ApuViewerPanel::drawChannel4Section(const NoiseChannel& channel, const AudioControl& control)
{
    drawSectionHeader("Channel 4 - Noise (NR41-NR44)");
    drawChannelStatusIndicator(channel.channelEnabled, channel.dacEnabled);
    ImGui::SameLine(0.f, 16.f);
    drawPanningIndicator(control.channel4Left, control.channel4Right);

    drawEnvelopeState(channel.initialVolume, channel.envelopeDirectionIncrease,
                      channel.envelopeSweepPace, channel.currentVolume, channel.envelopeTimer);
    drawLengthTimer(channel.lengthEnabled, channel.lengthTimer);

    ImGui::TextColored(NameColor, "Frequency:");
    ImGui::SameLine();
    ImGui::TextColored(ValueColor, "%.1f Hz  (shift: %u  divider: %u)",
        calculateNoiseFrequencyHz(channel.clockShift, channel.clockDivider),
        channel.clockShift, channel.clockDivider);

    ImGui::TextColored(NameColor, "LFSR mode:");
    ImGui::SameLine();
    ImGui::TextColored(ValueColor, channel.shortModeEnabled ? "7-bit" : "15-bit");
    ImGui::SameLine(0.f, 16.f);
    ImGui::TextColored(NameColor, "LFSR value:");
    ImGui::SameLine();
    ImGui::TextColored(ValueColor, "0x%04X", channel.noiseShiftRegister);
}

void ApuViewerPanel::drawMasterControlSection(const AudioControl& control)
{
    drawSectionHeader("Master Control (NR50-NR52)");

    ImGui::TextColored(NameColor, "APU:");
    ImGui::SameLine();
    ImGui::TextColored(control.masterEnabled ? EnabledColor : DisabledColor,
                       control.masterEnabled ? "enabled" : "disabled");

    ImGui::TextColored(NameColor, "Volume:");
    ImGui::SameLine();
    ImGui::TextColored(ValueColor, "L: %u  R: %u", control.leftVolume, control.rightVolume);

    if(ImGui::BeginTable("##panningTable", 3, ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("Channel", ImGuiTableColumnFlags_WidthFixed, 90.f);
        ImGui::TableSetupColumn("Left",    ImGuiTableColumnFlags_WidthFixed, 50.f);
        ImGui::TableSetupColumn("Right",   ImGuiTableColumnFlags_WidthFixed, 50.f);
        ImGui::TableHeadersRow();

        auto drawPanRow = [&](const char* channelName, bool left, bool right)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::TextColored(NameColor, "%s", channelName);
            ImGui::TableNextColumn(); ImGui::TextColored(left  ? EnabledColor : DisabledColor, left  ? "on" : "--");
            ImGui::TableNextColumn(); ImGui::TextColored(right ? EnabledColor : DisabledColor, right ? "on" : "--");
        };

        drawPanRow("Ch1 Pulse+Sw", control.channel1Left,  control.channel1Right);
        drawPanRow("Ch2 Pulse",    control.channel2Left,  control.channel2Right);
        drawPanRow("Ch3 Wave",     control.channel3Left,  control.channel3Right);
        drawPanRow("Ch4 Noise",    control.channel4Left,  control.channel4Right);

        ImGui::EndTable();
    }
}
