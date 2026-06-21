#include "ToolWindow.h"

#include <imgui.h>
#include <imgui-SFML.h>

#include "Common.h"

ToolWindow::ToolWindow(std::string windowTitle, unsigned int windowWidth, unsigned int windowHeight)
    : title(std::move(windowTitle)), width(windowWidth), height(windowHeight)
{
}

bool ToolWindow::isOpen() const
{
    return open;
}

void ToolWindow::setOpen(bool isOpenRequested)
{
    // Deferred: actually creating/destroying the window touches the global current ImGui context
    // (see update()/renderFrame()), which would corrupt the main window's ImGui state if done here,
    // mid-menu, while it is being drawn.
    openRequested = isOpenRequested;
}

void ToolWindow::createWindow(bool startVisible)
{
    if(window)
        return;

    window.emplace(sf::VideoMode({width, height}), title, sf::Style::Titlebar | sf::Style::Close);
    window->setVisible(startVisible);

    if(!ImGui::SFML::Init(*window))
        Log::print(LogLevel::Error, "Failed to initialize ImGui-SFML for the ", title, " window");
}

void ToolWindow::destroyWindow()
{
    if(!window)
        return;

    ImGui::SFML::Shutdown(*window);
    window.reset();
}

void ToolWindow::onOpenRequested()
{
    createWindow();
    open = true;
}

void ToolWindow::onClosed()
{
    open = false;
    destroyWindow();
}

void ToolWindow::update(GameBoyEmulator& emulator)
{
    if(openRequested && !open)
        onOpenRequested();
    else if(!openRequested && open)
        onClosed();

    if(!window)
        return;

    renderFrame(emulator);
}

void ToolWindow::renderFrame(GameBoyEmulator& emulator)
{
    while(const std::optional<sf::Event> event = window->pollEvent())
    {
        ImGui::SFML::ProcessEvent(*window, *event);

        if(event->is<sf::Event::Closed>())
        {
            openRequested = false;
            onClosed();
            return;
        }
    }

    ImGui::SFML::Update(*window, deltaClock.restart());

    window->clear();
    drawContent(emulator);
    ImGui::SFML::Render(*window);

    window->display();
}
