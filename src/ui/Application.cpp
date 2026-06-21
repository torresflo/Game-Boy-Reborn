#include "Application.h"

#include <algorithm>
#include <filesystem>
#include <format>

#include <imgui.h>
#include <imgui-SFML.h>

#include "Cartridge.h"
#include "Common.h"
#include "PixelProcessingUnit.h"

namespace
{
    // How many frames worth of real time can accumulate before being discarded,
    // so a long stall (eg. dragging the window) doesn't trigger a burst of catch-up frames.
    constexpr double MaxAccumulatedFrames = 5.0;
}

Application::Application()
    : window(sf::VideoMode({640u, 576u}), "Game-Boy-Reborn"), gameScreenSprite(gameScreenTexture)
{
    if(!ImGui::SFML::Init(window))
        Log::print(LogLevel::Error, "Failed to initialize ImGui-SFML");

    if(!gameScreenTexture.resize({PixelProcessingUnit::ScreenWidth, PixelProcessingUnit::ScreenHeight}))
        Log::print(LogLevel::Error, "Failed to create the game screen texture");

    gameScreenSprite.setTexture(gameScreenTexture, true);
    updateGameScreenTransform(window.getSize());

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

        sf::Time deltaTime = deltaClock.restart();
        ImGui::SFML::Update(window, deltaTime);

        updateEmulation(deltaTime);
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

        if(const sf::Event::Resized* resizedEvent = event->getIf<sf::Event::Resized>())
        {
            window.setView(sf::View(sf::FloatRect({0.f, 0.f}, static_cast<sf::Vector2f>(resizedEvent->size))));
            updateGameScreenTransform(resizedEvent->size);
        }
    }
}

void Application::updateEmulation(sf::Time deltaTime)
{
    if(!emulator.isROMLoaded() || emulator.isPaused())
    {
        frameTimeAccumulator = 0.0;
        return;
    }

    frameTimeAccumulator += deltaTime.asSeconds();

    double maxAccumulator = GameBoyEmulator::SecondsPerFrame * MaxAccumulatedFrames;
    if(frameTimeAccumulator > maxAccumulator)
        frameTimeAccumulator = maxAccumulator;

    while(frameTimeAccumulator >= GameBoyEmulator::SecondsPerFrame)
    {
        emulator.stepOneFrame();
        frameTimeAccumulator -= GameBoyEmulator::SecondsPerFrame;
    }
}

void Application::update()
{
    updateWindowTitle();

    drawMenuBar();

    romFileDialog.update(emulator);
    registerViewerWindow.update(emulator);
    cartridgeViewerWindow.update(emulator);
    tileDataViewerWindow.update(emulator);
    logViewerWindow.update(emulator);
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
                romFileDialog.setOpen(true);
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

            bool tileDataViewerOpen = tileDataViewerWindow.isOpen();
            if(ImGui::MenuItem("Tile Data", nullptr, &tileDataViewerOpen))
                tileDataViewerWindow.setOpen(tileDataViewerOpen);

            bool logViewerOpen = logViewerWindow.isOpen();
            if(ImGui::MenuItem("Log", nullptr, &logViewerOpen))
                logViewerWindow.setOpen(logViewerOpen);

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void Application::updateWindowTitle()
{
    if(emulator.isROMLoaded())
    {
        const std::string& romPath = emulator.getCartridge().getRomPath();
        if(romPath != romPathInWindowTitle)
        {
            window.setTitle(std::format("Game-Boy-Reborn - {}", std::filesystem::path(romPath).filename().string()));
            romPathInWindowTitle = romPath;
        }
    }
    else if(!romPathInWindowTitle.empty())
    {
        window.setTitle("Game-Boy-Reborn");
        romPathInWindowTitle.clear();
    }
}

void Application::render()
{
    const auto& frameBuffer = emulator.getPPU().getFrameBuffer();
    gameScreenTexture.update(reinterpret_cast<const std::uint8_t*>(frameBuffer.data()));
    window.draw(gameScreenSprite);
}

void Application::updateGameScreenTransform(sf::Vector2u windowSize)
{
    float scaleX = static_cast<float>(windowSize.x) / static_cast<float>(PixelProcessingUnit::ScreenWidth);
    float scaleY = static_cast<float>(windowSize.y) / static_cast<float>(PixelProcessingUnit::ScreenHeight);
    float scale = std::min(scaleX, scaleY);

    gameScreenSprite.setScale({scale, scale});

    sf::Vector2f scaledScreenSize{PixelProcessingUnit::ScreenWidth * scale, PixelProcessingUnit::ScreenHeight * scale};
    gameScreenSprite.setPosition({(static_cast<float>(windowSize.x) - scaledScreenSize.x) / 2.f,
                                   (static_cast<float>(windowSize.y) - scaledScreenSize.y) / 2.f});
}
