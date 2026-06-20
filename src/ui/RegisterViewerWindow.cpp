#include "RegisterViewerWindow.h"

#include <imgui.h>

#include "CentralProcessingUnit.h"

void RegisterViewerWindow::draw(const CentralProcessingUnit& cpu, bool& isOpen)
{
    if(!isOpen)
        return;

    if(!ImGui::Begin("CPU Registers", &isOpen))
    {
        ImGui::End();
        return;
    }

    const Registers& registers = cpu.getRegisters();

    if(ImGui::BeginTable("RegistersTable", 2))
    {
        ImGui::TableNextColumn(); ImGui::Text("A"); ImGui::TableNextColumn(); ImGui::Text("0x%02X", registers.A);
        ImGui::TableNextColumn(); ImGui::Text("F"); ImGui::TableNextColumn(); ImGui::Text("0x%02X", registers.F);
        ImGui::TableNextColumn(); ImGui::Text("B"); ImGui::TableNextColumn(); ImGui::Text("0x%02X", registers.B);
        ImGui::TableNextColumn(); ImGui::Text("C"); ImGui::TableNextColumn(); ImGui::Text("0x%02X", registers.C);
        ImGui::TableNextColumn(); ImGui::Text("D"); ImGui::TableNextColumn(); ImGui::Text("0x%02X", registers.D);
        ImGui::TableNextColumn(); ImGui::Text("E"); ImGui::TableNextColumn(); ImGui::Text("0x%02X", registers.E);
        ImGui::TableNextColumn(); ImGui::Text("H"); ImGui::TableNextColumn(); ImGui::Text("0x%02X", registers.H);
        ImGui::TableNextColumn(); ImGui::Text("L"); ImGui::TableNextColumn(); ImGui::Text("0x%02X", registers.L);
        ImGui::TableNextColumn(); ImGui::Text("HL"); ImGui::TableNextColumn(); ImGui::Text("0x%04X", registers.getHL());
        ImGui::TableNextColumn(); ImGui::Text("SP"); ImGui::TableNextColumn(); ImGui::Text("0x%04X", registers.SP);
        ImGui::TableNextColumn(); ImGui::Text("PC"); ImGui::TableNextColumn(); ImGui::Text("0x%04X", registers.PC);
        ImGui::EndTable();
    }

    ImGui::Separator();
    ImGui::Text("Flags: %c%c%c%c",
        cpu.flagZ() ? 'Z' : '-',
        cpu.flagN() ? 'N' : '-',
        cpu.flagH() ? 'H' : '-',
        cpu.flagC() ? 'C' : '-');

    ImGui::Text("IME: %s", cpu.isInterruptMasterEnabled() ? "enabled" : "disabled");
    ImGui::Text("Halted: %s", cpu.isHalted() ? "yes" : "no");
    ImGui::Text("Cycles: %llu", static_cast<unsigned long long>(cpu.getCycleCount()));

    ImGui::End();
}
