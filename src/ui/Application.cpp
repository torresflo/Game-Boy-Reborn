#include "Application.h"

#include <imgui.h>
#include <imgui-SFML.h>

#include "Common.h"
#include "PixelProcessingUnit.h"

Application::Application()
    : window(sf::VideoMode({640u, 576u}), "Game-Boy-Reborn"), gameScreenSprite(gameScreenTexture)
{
    if(!ImGui::SFML::Init(window))
        Log::print(LogLevel::Error, "Failed to initialize ImGui-SFML");

    if(!gameScreenTexture.resize({PixelProcessingUnit::ScreenWidth, PixelProcessingUnit::ScreenHeight}))
        Log::print(LogLevel::Error, "Failed to create the game screen texture");

    gameScreenSprite.setTexture(gameScreenTexture, true);
    gameScreenSprite.setScale({4.f, 4.f});

    window.setFramerateLimit(60);
}

Application::~Application()
{
    ImGui::SFML::Shutdown();
}

void Application::run()
{
    while(window.isOpen())
    {
        processEvents();
        ImGui::SFML::Update(window, deltaClock.restart());

        update();

        window.clear();
        render();

        ImGui::SFML::Render(window);
        window.display();
    }
}

void Application::processEvents()
{
    while(const std::optional<sf::Event> event = window.pollEvent())
    {
        ImGui::SFML::ProcessEvent(window, *event);

        if(event->is<sf::Event::Closed>())
            window.close();
    }
}

void Application::update()
{
    if(emulator.isROMLoaded() && !emulator.isPaused())
        emulator.stepOneFrame();

    drawMenuBar();

    // These manage their own dedicated SFML window and ImGui context, and must
    // run last so they don't leave a non-main context active for the code above.
    romFileDialog.update(emulator);
    registerViewerWindow.update(emulator.getCPU());
    cartridgeViewerWindow.update(emulator.getCartridge());
}

void Application::drawMenuBar()
{
    if(ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("Open ROM..."))
            {
                emulator.setPaused(true);
                romFileDialog.openDialog();
            }

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Emulation"))
        {
            bool paused = emulator.isPaused();
            if(ImGui::MenuItem("Pause", nullptr, &paused, emulator.isROMLoaded()))
                emulator.setPaused(paused);

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Debug"))
        {
            bool registerViewerOpen = registerViewerWindow.isOpen();
            if(ImGui::MenuItem("CPU Registers", nullptr, &registerViewerOpen))
                registerViewerWindow.setOpen(registerViewerOpen);

            bool cartridgeViewerOpen = cartridgeViewerWindow.isOpen();
            if(ImGui::MenuItem("Cartridge Info", nullptr, &cartridgeViewerOpen))
                cartridgeViewerWindow.setOpen(cartridgeViewerOpen);

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void Application::render()
{
    const auto& frameBuffer = emulator.getPPU().getFrameBuffer();
    gameScreenTexture.update(reinterpret_cast<const std::uint8_t*>(frameBuffer.data()));
    window.draw(gameScreenSprite);
}
