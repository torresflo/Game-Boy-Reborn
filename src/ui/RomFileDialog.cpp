#include "RomFileDialog.h"

#include <imgui.h>
#include <imgui-SFML.h>
#include <ImGuiFileDialog.h>

#include "Common.h"
#include "Emulator.h"

void RomFileDialog::openDialog()
{
    // Deferred: actually creating/destroying the window touches the global current ImGui context (see update()),
    // which would corrupt the main window's ImGui state if done here, mid-menu, while it is being drawn.
    pendingOpen = true;
}

void RomFileDialog::update(Emulator& emulator)
{
    if(pendingOpen)
    {
        pendingOpen = false;
        ensureWindowCreated();
        window->setVisible(true);
        open = true;

        ImGuiFileDialog::Instance()->OpenDialog(DialogKey, "Choose a ROM", ".gb,.gbc", IGFD::FileDialogConfig{});
    }

    if(!open)
        return;

    while(const std::optional<sf::Event> event = window->pollEvent())
    {
        ImGui::SFML::ProcessEvent(*window, *event);

        if(event->is<sf::Event::Closed>())
        {
            ImGuiFileDialog::Instance()->Close();
            window->setVisible(false);
            open = false;
            return;
        }
    }

    ImGui::SFML::Update(*window, deltaClock.restart());

    const ImVec2 windowSize(static_cast<float>(WindowWidth), static_cast<float>(WindowHeight));
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));

    const bool dialogClosed = ImGuiFileDialog::Instance()->Display(DialogKey, ImGuiWindowFlags_NoCollapse, windowSize, windowSize);
    if(dialogClosed && ImGuiFileDialog::Instance()->IsOk())
        emulator.loadROM(ImGuiFileDialog::Instance()->GetFilePathName());

    window->clear();
    ImGui::SFML::Render(*window);
    window->display();

    if(dialogClosed)
    {
        ImGuiFileDialog::Instance()->Close();
        window->setVisible(false);
        open = false;
    }
}

void RomFileDialog::ensureWindowCreated()
{
    if(window)
        return;

    window.emplace(sf::VideoMode({WindowWidth, WindowHeight}), "Open ROM", sf::Style::Titlebar | sf::Style::Close);
    window->setVisible(false);

    if(!ImGui::SFML::Init(*window))
        Log::print(LogLevel::Error, "Failed to initialize ImGui-SFML for the Open ROM window");
}