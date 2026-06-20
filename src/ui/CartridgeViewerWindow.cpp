#include "CartridgeViewerWindow.h"

#include <format>

#include <imgui.h>
#include <imgui-SFML.h>

#include "Cartridge.h"
#include "Common.h"

bool CartridgeViewerWindow::isOpen() const
{
    return open;
}

void CartridgeViewerWindow::setOpen(bool isOpenRequested)
{
    // Deferred: actually creating/destroying the window touches the global current ImGui context (see update()),
    // which would corrupt the main window's ImGui state if done here, mid-menu, while it is being drawn.
    open = isOpenRequested;
}

void CartridgeViewerWindow::createWindow()
{
    window.emplace(sf::VideoMode({WindowWidth, WindowHeight}), "Cartridge Info", sf::Style::Titlebar | sf::Style::Close);

    if(!ImGui::SFML::Init(*window))
        Log::print(LogLevel::Error, "Failed to initialize ImGui-SFML for the Cartridge Info window");
}

void CartridgeViewerWindow::closeWindow()
{
    if(!window)
        return;

    ImGui::SFML::Shutdown(*window);
    window.reset();
}

void CartridgeViewerWindow::update(const Cartridge& cartridge)
{
    if(open && !window)
        createWindow();
    else if(!open && window)
        closeWindow();

    if(!window)
        return;

    while(const std::optional<sf::Event> event = window->pollEvent())
    {
        ImGui::SFML::ProcessEvent(*window, *event);

        if(event->is<sf::Event::Closed>())
        {
            open = false;
            closeWindow();
            return;
        }
    }

    ImGui::SFML::Update(*window, deltaClock.restart());

    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(WindowWidth), static_cast<float>(WindowHeight)));
    ImGui::Begin("Cartridge Info", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    const auto& header = cartridge.getHeader();

    ImGui::Text("Title       : %s", header.title);
    ImGui::Text("Type        : %s (%s)", std::format("{:02X}", header.cartridgeType).c_str(), cartridge.getRomTypeName(header.cartridgeType).c_str());
    ImGui::Text("ROM Size    : %d KB", 32 << header.romSize);
    ImGui::Text("RAM Size    : %s", std::format("{:02X}", header.ramSize).c_str());
    ImGui::Text("Licence     : 0x%s (%s)", std::format("{:02X}", header.oldLicenceCode).c_str(), cartridge.getLicenceName(header.oldLicenceCode).c_str());
    ImGui::Text("ROM Version : %s", std::format("{:02X}", header.maskRomVersion).c_str());
    ImGui::Text("Checksum    : %s (%s)", std::format("{:02X}", header.headerChecksum).c_str(), cartridge.checkHeaderChecksum() ? "PASSED" : "FAILED");

    ImGui::End();

    window->clear();
    ImGui::SFML::Render(*window);
    window->display();
}
