#include "RegisterViewerWindow.h"

#include <imgui.h>
#include <imgui-SFML.h>

#include "CentralProcessingUnit.h"
#include "Common.h"

bool RegisterViewerWindow::isOpen() const
{
    return open;
}

void RegisterViewerWindow::setOpen(bool isOpenRequested)
{
    // Deferred: actually creating/destroying the window touches the global current ImGui context (see update()),
    // which would corrupt the main window's ImGui state if done here, mid-menu, while it is being drawn.
    open = isOpenRequested;
}

void RegisterViewerWindow::createWindow()
{
    window.emplace(sf::VideoMode({WindowWidth, WindowHeight}), "CPU Registers", sf::Style::Titlebar | sf::Style::Close);

    if(!ImGui::SFML::Init(*window))
        Log::print(LogLevel::Error, "Failed to initialize ImGui-SFML for the CPU Registers window");
}

void RegisterViewerWindow::closeWindow()
{
    if(!window)
        return;

    ImGui::SFML::Shutdown(*window);
    window.reset();
}

void RegisterViewerWindow::update(const CentralProcessingUnit& cpu)
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
    ImGui::Begin("CPU Registers", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

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

    window->clear();
    ImGui::SFML::Render(*window);
    window->display();
}
