#include "RegisterViewerWindow.h"

#include <format>

#include "CentralProcessingUnit.h"
#include "GameBoyEmulator.h"
#include "MemoryBus.h"
#include "PixelProcessingUnit.h"

RegisterViewerWindow::RegisterViewerWindow()
    : ToolWindow("CPU Registers", WindowWidth, WindowHeight)
{
}

void RegisterViewerWindow::drawSectionHeader(const char* label)
{
    ImGui::Spacing();
    ImGui::TextColored(HeaderColor, "%s", label);
    ImGui::Separator();
}

bool RegisterViewerWindow::beginRegisterTable(const char* id)
{
    if(!ImGui::BeginTable(id, 4, ImGuiTableFlags_SizingFixedFit))
        return false;

    ImGui::TableSetupColumn("##Name1", ImGuiTableColumnFlags_WidthFixed, RegisterNameColumnWidth);
    ImGui::TableSetupColumn("##Value1", ImGuiTableColumnFlags_WidthFixed, RegisterValueColumnWidth);
    ImGui::TableSetupColumn("##Name2", ImGuiTableColumnFlags_WidthFixed, RegisterNameColumnWidth);
    ImGui::TableSetupColumn("##Value2", ImGuiTableColumnFlags_WidthFixed, RegisterValueColumnWidth);
    return true;
}

void RegisterViewerWindow::drawRegisterCell(const char* name, const std::string& valueText)
{
    ImGui::TableNextColumn();
    ImGui::TextColored(NameColor, "%s", name);
    ImGui::TableNextColumn();
    ImGui::TextColored(ValueColor, "%s", valueText.c_str());
}

void RegisterViewerWindow::drawFlag(const char* label, bool set)
{
    ImGui::TextColored(set ? FlagSetColor : FlagClearColor, "%s", label);
}

void RegisterViewerWindow::drawInfoLine(const char* name, const std::string& valueText)
{
    ImGui::TextColored(NameColor, "%s", name);
    ImGui::SameLine();
    ImGui::TextColored(ValueColor, "%s", valueText.c_str());
}

void RegisterViewerWindow::drawContent(GameBoyEmulator& emulator)
{
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(WindowWidth), static_cast<float>(WindowHeight)));
    ImGui::Begin("CPU Registers", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    const CentralProcessingUnit& cpu = emulator.getCPU();
    const Registers& registers = cpu.getRegisters();
    const PixelProcessingUnit& ppu = emulator.getPPU();
    const MemoryBus& bus = emulator.getMemoryBus();

    drawSectionHeader("16-bit Registers");
    if(beginRegisterTable("Registers16Table"))
    {
        drawRegisterCell("AF", std::format("0x{:04X}", registers.getAF()));
        drawRegisterCell("BC", std::format("0x{:04X}", registers.getBC()));
        drawRegisterCell("DE", std::format("0x{:04X}", registers.getDE()));
        drawRegisterCell("HL", std::format("0x{:04X}", registers.getHL()));
        drawRegisterCell("SP", std::format("0x{:04X}", registers.SP));
        drawRegisterCell("PC", std::format("0x{:04X}", registers.PC));
        ImGui::EndTable();
    }

    drawSectionHeader("8-bit Registers");
    if(beginRegisterTable("Registers8Table"))
    {
        drawRegisterCell("A", std::format("0x{:02X}", registers.A));
        drawRegisterCell("F", std::format("0x{:02X}", registers.F));
        drawRegisterCell("B", std::format("0x{:02X}", registers.B));
        drawRegisterCell("C", std::format("0x{:02X}", registers.C));
        drawRegisterCell("D", std::format("0x{:02X}", registers.D));
        drawRegisterCell("E", std::format("0x{:02X}", registers.E));
        drawRegisterCell("H", std::format("0x{:02X}", registers.H));
        drawRegisterCell("L", std::format("0x{:02X}", registers.L));
        ImGui::EndTable();
    }

    drawSectionHeader("Flags");
    drawFlag("Z", cpu.flagZ());
    ImGui::SameLine();
    drawFlag("N", cpu.flagN());
    ImGui::SameLine();
    drawFlag("H", cpu.flagH());
    ImGui::SameLine();
    drawFlag("C", cpu.flagC());

    drawSectionHeader("PPU Registers");
    if(beginRegisterTable("PPURegistersTable"))
    {
        drawRegisterCell("LCDC", std::format("0x{:02X}", ppu.readRegister(0xFF40)));
        drawRegisterCell("STAT", std::format("0x{:02X}", ppu.readRegister(0xFF41)));
        drawRegisterCell("SCY",  std::format("0x{:02X}", ppu.readRegister(0xFF42)));
        drawRegisterCell("SCX",  std::format("0x{:02X}", ppu.readRegister(0xFF43)));
        drawRegisterCell("LY",   std::format("0x{:02X}", ppu.readRegister(0xFF44)));
        drawRegisterCell("LYC",  std::format("0x{:02X}", ppu.readRegister(0xFF45)));
        drawRegisterCell("WY",   std::format("0x{:02X}", ppu.readRegister(0xFF4A)));
        drawRegisterCell("WX",   std::format("0x{:02X}", ppu.readRegister(0xFF4B)));
        drawRegisterCell("BGP",  std::format("0x{:02X}", ppu.readRegister(0xFF47)));
        drawRegisterCell("OBP0", std::format("0x{:02X}", ppu.readRegister(0xFF48)));
        drawRegisterCell("OBP1", std::format("0x{:02X}", ppu.readRegister(0xFF49)));
        ImGui::EndTable();
    }

    drawSectionHeader("Timer Registers");
    if(beginRegisterTable("TimerRegistersTable"))
    {
        drawRegisterCell("DIV",  std::format("0x{:02X}", bus.read(0xFF04)));
        drawRegisterCell("TIMA", std::format("0x{:02X}", bus.read(0xFF05)));
        drawRegisterCell("TMA",  std::format("0x{:02X}", bus.read(0xFF06)));
        drawRegisterCell("TAC",  std::format("0x{:02X}", bus.read(0xFF07)));
        ImGui::EndTable();
    }

    drawSectionHeader("Interrupt Registers");
    if(beginRegisterTable("InterruptRegistersTable"))
    {
        drawRegisterCell("IE", std::format("0x{:02X}", bus.readInterruptEnableRegister()));
        drawRegisterCell("IF", std::format("0x{:02X}", bus.readInterruptFlags()));
        ImGui::EndTable();
    }

    drawSectionHeader("Execution State");
    drawInfoLine("IME", cpu.isInterruptMasterEnabled() ? "enabled" : "disabled");
    drawInfoLine("Halted", cpu.isHalted() ? "yes" : "no");
    drawInfoLine("Cycles", std::format("{}", cpu.getCycleCount()));

    ImGui::End();
}
