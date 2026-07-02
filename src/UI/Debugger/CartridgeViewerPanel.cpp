#include "CartridgeViewerPanel.h"

#include <format>

#include <imgui.h>

#include "Cartridge.h"
#include "GameBoyEmulator.h"

CartridgeViewerPanel::CartridgeViewerPanel()
    : DebugPanel("Cartridge Info")
{
}

void CartridgeViewerPanel::draw(GameBoyEmulator& emulator)
{
    if(emulator.isROMLoaded())
    {
        const Cartridge& cartridge = emulator.getCartridge();
        const auto& header = cartridge.getHeader();

        ImGui::Text("Title       : %s", header.title);
        ImGui::Text("Type        : %s (%s)", std::format("{:02X}", header.cartridgeType).c_str(), cartridge.getRomTypeName(header.cartridgeType).c_str());
        ImGui::Text("ROM Size    : %d KB", 32 << header.romSize);
        ImGui::Text("RAM Size    : %s", std::format("{:02X}", header.ramSize).c_str());
        ImGui::Text("Licence     : 0x%s (%s)", std::format("{:02X}", header.oldLicenceCode).c_str(), cartridge.getLicenceName(header.oldLicenceCode).c_str());
        ImGui::Text("ROM Version : %s", std::format("{:02X}", header.maskRomVersion).c_str());
        ImGui::Text("Checksum    : %s (%s)", std::format("{:02X}", header.headerChecksum).c_str(), cartridge.checkHeaderChecksum() ? "PASSED" : "FAILED");
    }
    else
    {
        ImGui::Text("No Rom loaded");
    }
}
