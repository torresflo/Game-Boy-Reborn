#include "StackViewerPanel.h"

#include <algorithm>

#include "GameBoyEmulator.h"
#include "MemoryBus.h"
#include "CentralProcessingUnit.h"

StackViewerPanel::StackViewerPanel()
    : DebugPanel("Stack")
{
}

void StackViewerPanel::draw(GameBoyEmulator& emulator)
{
    const u16 stackPointer = emulator.getCPU().getRegisters().SP;
    const MemoryBus& bus = emulator.getMemoryBus();

    ImGui::TextColored(HeaderColor, "SP");
    ImGui::SameLine();
    ImGui::TextColored(ValueColor, "0x%04X", stackPointer);

    ImGui::SetNextItemWidth(120.f);
    if(ImGui::InputInt("Words", &stackWordCount))
        stackWordCount = std::clamp(stackWordCount, MinimumStackWordCount, MaximumStackWordCount);

    constexpr ImGuiTableFlags tableFlags =
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_SizingFixedFit |
        ImGuiTableFlags_BordersInnerV;

    if(ImGui::BeginTable("##stack", 2, tableFlags))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 70.f);
        ImGui::TableSetupColumn("Word", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        for(int index = 0; index < stackWordCount; ++index)
        {
            const u32 address = static_cast<u32>(stackPointer) + static_cast<u32>(index) * 2;
            if(address + 1 > 0xFFFF)
                break;

            const u16 low = bus.read(static_cast<u16>(address));
            const u16 high = bus.read(static_cast<u16>(address + 1));
            const u16 word = static_cast<u16>((high << 8) | low);

            ImGui::TableNextRow();

            if(index == 0)
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
                    ImGui::ColorConvertFloat4ToU32(StackPointerHighlightColor));

            ImGui::TableNextColumn();
            ImGui::TextColored(AddressColor, "0x%04X", static_cast<u16>(address));

            ImGui::TableNextColumn();
            ImGui::TextColored(ValueColor, "0x%04X", word);
        }

        ImGui::EndTable();
    }
}
