#include "BreakpointPanel.h"

#include <format>
#include <optional>

#include "GameBoyEmulator.h"
#include "CentralProcessingUnit.h"

BreakpointPanel::BreakpointPanel()
    : DebugPanel("Breakpoints")
{
}

void BreakpointPanel::draw(GameBoyEmulator& emulator)
{
    drawExecutionControls(emulator);
    ImGui::Separator();
    drawBreakpointList(emulator);
}

void BreakpointPanel::drawExecutionControls(GameBoyEmulator& emulator)
{
    ImGui::TextColored(HeaderColor, "Execution");

    ImGui::BeginDisabled(!emulator.isROMLoaded());

    if(emulator.isPaused())
    {
        if(ImGui::Button("Run (F5)"))
            emulator.setPaused(false);
    }
    else
    {
        if(ImGui::Button("Pause (F5)"))
            emulator.setPaused(true);
    }

    ImGui::SameLine();
    if(ImGui::Button("Step (F10)"))
    {
        emulator.setPaused(true);
        emulator.stepOneInstruction();
    }

    ImGui::SameLine();
    if(ImGui::Button("Step Frame (F6)"))
    {
        emulator.setPaused(true);
        emulator.stepOneFrame();
    }

    ImGui::EndDisabled();

    const u16 programCounter = emulator.getCPU().getRegisters().PC;
    ImGui::TextColored(AddressColor, "PC");
    ImGui::SameLine();
    ImGui::TextColored(ValueColor, "0x%04X", programCounter);
    ImGui::SameLine();
    ImGui::TextColored(AddressColor, "  |  %s", emulator.isPaused() ? "Paused" : "Running");
}

void BreakpointPanel::drawBreakpointList(GameBoyEmulator& emulator)
{
    ImGui::TextColored(HeaderColor, "Breakpoints");

    ImGui::SetNextItemWidth(80.f);
    const bool addFromInput = ImGui::InputScalar("##address", ImGuiDataType_U16, &breakpointInputAddress,
        nullptr, nullptr, "%04X",
        ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    if(ImGui::Button("Add") || addFromInput)
        emulator.toggleBreakpoint(breakpointInputAddress);

    ImGui::SameLine();
    if(ImGui::Button("Clear all"))
        emulator.clearBreakpoints();

    const u16 programCounter = emulator.getCPU().getRegisters().PC;

    constexpr ImGuiTableFlags tableFlags =
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_SizingFixedFit |
        ImGuiTableFlags_BordersInnerV;

    if(ImGui::BeginTable("##breakpoints", 2, tableFlags))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##remove", ImGuiTableColumnFlags_WidthFixed, 30.f);
        ImGui::TableHeadersRow();

        std::optional<u16> addressToRemove;

        for(u16 address : emulator.getBreakpoints())
        {
            ImGui::TableNextRow();

            if(address == programCounter)
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
                    ImGui::ColorConvertFloat4ToU32(CurrentBreakpointHighlightColor));

            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Text, BreakpointColor);
            ImGui::Bullet();
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::TextColored(ValueColor, "0x%04X", address);

            ImGui::TableNextColumn();
            ImGui::PushID(address);
            if(ImGui::SmallButton("X"))
                addressToRemove = address;
            ImGui::PopID();
        }

        ImGui::EndTable();

        if(addressToRemove.has_value())
            emulator.removeBreakpoint(addressToRemove.value());
    }
}
