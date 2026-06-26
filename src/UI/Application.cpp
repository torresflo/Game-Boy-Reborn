#include "Application.h"

#include <algorithm>
#include <filesystem>
#include <format>

#include <imgui.h>
#include <imgui-SFML.h>

#include "Cartridge.h"
#include "Common.h"
#include "PixelProcessingUnit.h"

Application* Application::instancePointer = nullptr;

Application::Application()
    : gameScreenSprite(gameScreenTexture)
{
    instancePointer = this;

    createWindow();

    if(!ImGui::SFML::Init(window))
        Log::print(LogLevel::Error, "Failed to initialize ImGui-SFML");

    if(!gameScreenTexture.resize({PixelProcessingUnit::ScreenWidth, PixelProcessingUnit::ScreenHeight}))
        Log::print(LogLevel::Error, "Failed to create the game screen texture");

    gameScreenSprite.setTexture(gameScreenTexture, true);
    updateGameScreenTransform(window.getSize());

    audioStream.initializeStream();
    audioStream.play();
}

Application::~Application()
{
    ImGui::SFML::Shutdown();
    instancePointer = nullptr;
}

Application& Application::instance()
{
    return *instancePointer;
}

NotificationManager& Application::getNotificationManager()
{
    return notificationManager;
}

void Application::run()
{
    while(window.isOpen())
    {
        processEvents();
        updateGamepadInput();

        sf::Time deltaTime = deltaClock.restart();
        ImGui::SFML::Update(window, deltaTime);

        updateEmulation(deltaTime);
        update(deltaTime);

        window.clear();
        render();

        ImGui::SFML::Render(window);
        window.display();
    }
}

void Application::createWindow()
{
    window.close();
    sf::VideoMode videoMode = (windowState == sf::State::Fullscreen ? sf::VideoMode({1920, 1080}) : sf::VideoMode({windowWidth, windowHeight}));
    window.create(videoMode, "Game-Boy-Reborn", windowState);
    window.setFramerateLimit(60);
    romPathInWindowTitle.clear(); //Clear so it is updated next frame
}

void Application::processEvents()
{
    while(const std::optional<sf::Event> event = window.pollEvent())
    {
        ImGui::SFML::ProcessEvent(window, *event);

        if(event->is<sf::Event::Closed>())
            window.close();
        else if(const sf::Event::Resized* resizedEvent = event->getIf<sf::Event::Resized>())
            window.setView(sf::View(sf::FloatRect({0.f, 0.f}, static_cast<sf::Vector2f>(resizedEvent->size))));
        else if(const sf::Event::KeyPressed* keyPressedEvent = event->getIf<sf::Event::KeyPressed>())
            processKeyPressedEvent(*keyPressedEvent);
        else if(const sf::Event::KeyReleased* keyReleasedEvent = event->getIf<sf::Event::KeyReleased>())
            processKeyReleasedEvent(*keyReleasedEvent);
    }
}

void Application::processKeyPressedEvent(const sf::Event::KeyPressed &key)
{
    switch(key.code)
    {
        case sf::Keyboard::Key::F1:
        {
            menuBarVisible = !menuBarVisible;
            break;
        }
        case sf::Keyboard::Key::F11:
        {
            windowState = (windowState == sf::State::Windowed ? sf::State::Fullscreen : sf::State::Windowed);
            createWindow();
            break;
        }
        case sf::Keyboard::Key::Pause:
        {
            emulator.setPaused(!emulator.isPaused());
            break;
        }
        case sf::Keyboard::Key::Subtract:
        {
            cycleSpeedPreset(-1);
            break;
        }
        case sf::Keyboard::Key::Add:
        {
            cycleSpeedPreset(1);
            break;
        }
        case sf::Keyboard::Key::Equal:
        {
            setSpeedMultiplier(1.0f);
            break;
        }
        case sf::Keyboard::Key::F5:
        {
            if(emulator.isROMLoaded())
            {
                std::string fileName = std::filesystem::path(emulator.getQuickSaveStatePath()).filename().string();
                if(emulator.quickSaveState())
                    notificationManager.push(NotificationLevel::Info, "State saved: " + fileName);
                else
                    notificationManager.push(NotificationLevel::Error, "Failed to save state: " + fileName);
            }
            break;
        }
        case sf::Keyboard::Key::F9:
        {
            if(emulator.isROMLoaded())
            {
                std::string fileName = std::filesystem::path(emulator.getQuickSaveStatePath()).filename().string();
                if(emulator.quickLoadState())
                    notificationManager.push(NotificationLevel::Info, "State loaded: " + fileName);
                else
                    notificationManager.push(NotificationLevel::Error, "Failed to load state: " + fileName);
            }
            break;
        }
        default:
        {
            std::optional<Gamepad::Button> button = convertSFMLKeyboardKey(key.code);
            if(button.has_value())
                keyboardButtonStates[static_cast<u32>(button.value())] = true;
            break;
        }
    }
}

void Application::processKeyReleasedEvent(const sf::Event::KeyReleased &key)
{
    std::optional<Gamepad::Button> button = convertSFMLKeyboardKey(key.code);
    if(button.has_value())
        keyboardButtonStates[static_cast<u32>(button.value())] = false;
}

void Application::updateGamepadInput()
{
    joystickButtonStates.fill(false);

    for(unsigned int joystickId = 0u; joystickId < sf::Joystick::Count; ++joystickId)
    {
        if(!sf::Joystick::isConnected(joystickId))
            continue;

        for(unsigned int buttonIndex = 0u; buttonIndex < sf::Joystick::getButtonCount(joystickId); ++buttonIndex)
        {
            std::optional<Gamepad::Button> button = convertSFMLJoystickButton(buttonIndex);
            if(button.has_value() && sf::Joystick::isButtonPressed(joystickId, buttonIndex))
                joystickButtonStates[static_cast<u32>(button.value())] = true;
        }

        // The D-pad (PovX/PovY) and the left stick (X/Y) report up as opposite signs on Windows
        // PovY is positive going up, while the stick Y axis is negative going up.
        applyJoystickAxisDirection(sf::Joystick::getAxisPosition(joystickId, sf::Joystick::Axis::X), Gamepad::Button::Left, Gamepad::Button::Right);
        applyJoystickAxisDirection(sf::Joystick::getAxisPosition(joystickId, sf::Joystick::Axis::Y), Gamepad::Button::Up, Gamepad::Button::Down);
        applyJoystickAxisDirection(sf::Joystick::getAxisPosition(joystickId, sf::Joystick::Axis::PovX), Gamepad::Button::Left, Gamepad::Button::Right);
        applyJoystickAxisDirection(sf::Joystick::getAxisPosition(joystickId, sf::Joystick::Axis::PovY), Gamepad::Button::Down, Gamepad::Button::Up);
    }

    for(u32 buttonIndex = 0u; buttonIndex < Gamepad::ButtonCount; ++buttonIndex)
    {
        auto button = static_cast<Gamepad::Button>(buttonIndex);
        emulator.getGamepad().setButtonState(button, keyboardButtonStates[buttonIndex] || joystickButtonStates[buttonIndex]);
    }
}

void Application::applyJoystickAxisDirection(float axisPosition, Gamepad::Button negativeButton, Gamepad::Button positiveButton)
{
    if(axisPosition < -JoystickAxisThreshold)
        joystickButtonStates[static_cast<u32>(negativeButton)] = true;
    else if(axisPosition > JoystickAxisThreshold)
        joystickButtonStates[static_cast<u32>(positiveButton)] = true;
}

void Application::setSpeedMultiplier(float multiplier)
{
    speedMultiplier = multiplier;
    audioStream.setPitch(multiplier);
}

void Application::cycleSpeedPreset(int direction)
{
    auto current = std::find(SpeedPresets.begin(), SpeedPresets.end(), speedMultiplier);
    auto currentIndex = (current != SpeedPresets.end()) ? std::distance(SpeedPresets.begin(), current) : SpeedPresets.size() / 2;

    auto newIndex = std::clamp<std::ptrdiff_t>(currentIndex + direction, 0, static_cast<std::ptrdiff_t>(SpeedPresets.size()) - 1);
    setSpeedMultiplier(SpeedPresets[static_cast<std::size_t>(newIndex)]);
}

void Application::updateEmulation(sf::Time deltaTime)
{
    if(!emulator.isROMLoaded() || emulator.isPaused())
    {
        frameTimeAccumulator = 0.0;
        return;
    }

    double maxRealDeltaSeconds = GameBoyEmulator::SecondsPerFrame * MaxAccumulatedFrames;
    double realDeltaSeconds = std::min(static_cast<double>(deltaTime.asSeconds()), maxRealDeltaSeconds);

    frameTimeAccumulator += realDeltaSeconds * speedMultiplier;

    while(frameTimeAccumulator >= GameBoyEmulator::SecondsPerFrame)
    {
        emulator.stepOneFrame();
        audioRingBuffer.push(emulator.drainAudioSamples());
        frameTimeAccumulator -= GameBoyEmulator::SecondsPerFrame;
    }
}

void Application::update(sf::Time deltaTime)
{
    updateWindowTitle();

    drawMenuBar();
    updateGameScreenTransform(window.getSize());

    romFileDialog.update(emulator);
    saveStateFileDialog.update(emulator);
    loadStateFileDialog.update(emulator);
    registerViewerWindow.update(emulator);
    cartridgeViewerWindow.update(emulator);
    tileDataViewerWindow.update(emulator);
    objectViewerWindow.update(emulator);
    logViewerWindow.update(emulator);

    notificationManager.update(deltaTime);
    notificationManager.draw(window, menuBarHeight);
}

void Application::drawMenuBar()
{
    menuBarHeight = 0.f;

    if(!menuBarVisible)
        return;

    if(ImGui::BeginMainMenuBar())
    {
        menuBarHeight = ImGui::GetWindowSize().y;

        if(ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("Open ROM..."))
            {
                emulator.setPaused(true);
                romFileDialog.setOpen(true);
            }

            ImGui::Separator();

            if(ImGui::MenuItem("Save State As...", nullptr, false, emulator.isROMLoaded()))
            {
                emulator.setPaused(true);
                saveStateFileDialog.setOpen(true);
            }

            if(ImGui::MenuItem("Load State...", nullptr, false, emulator.isROMLoaded()))
            {
                emulator.setPaused(true);
                loadStateFileDialog.setOpen(true);
            }

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Emulation"))
        {
            bool paused = emulator.isPaused();
            if(ImGui::MenuItem("Pause", nullptr, &paused, emulator.isROMLoaded()))
                emulator.setPaused(paused);

            if(ImGui::BeginMenu("Speed"))
            {
                for(float preset : SpeedPresets)
                {
                    bool selected = (speedMultiplier == preset);
                    if(ImGui::MenuItem(std::format("{:g}x", preset).c_str(), nullptr, selected))
                        setSpeedMultiplier(preset);
                }

                ImGui::EndMenu();
            }

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

            bool objectViewerOpen = objectViewerWindow.isOpen();
            if(ImGui::MenuItem("Objects", nullptr, &objectViewerOpen))
                objectViewerWindow.setOpen(objectViewerOpen);

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
    float availableHeight = std::max(static_cast<float>(windowSize.y) - menuBarHeight, 0.f);

    float scaleX = static_cast<float>(windowSize.x) / static_cast<float>(PixelProcessingUnit::ScreenWidth);
    float scaleY = availableHeight / static_cast<float>(PixelProcessingUnit::ScreenHeight);
    float scale = std::min(scaleX, scaleY);

    gameScreenSprite.setScale({scale, scale});

    sf::Vector2f scaledScreenSize{PixelProcessingUnit::ScreenWidth * scale, PixelProcessingUnit::ScreenHeight * scale};
    gameScreenSprite.setPosition({(static_cast<float>(windowSize.x) - scaledScreenSize.x) / 2.f,
                                   menuBarHeight + (availableHeight - scaledScreenSize.y) / 2.f});
}

std::optional<Gamepad::Button> Application::convertSFMLKeyboardKey(const sf::Keyboard::Key& key) const
{
    switch(key)
    {
        case sf::Keyboard::Key::Up:
            return Gamepad::Button::Up;
        case sf::Keyboard::Key::Down:
            return Gamepad::Button::Down;
        case sf::Keyboard::Key::Left:
            return Gamepad::Button::Left;
        case sf::Keyboard::Key::Right:
            return Gamepad::Button::Right;
        case sf::Keyboard::Key::Enter:
            return Gamepad::Button::A;
        case sf::Keyboard::Key::Backspace:
            return Gamepad::Button::B;
        case sf::Keyboard::Key::Escape:
            return Gamepad::Button::Start;
        case sf::Keyboard::Key::Tab:
            return Gamepad::Button::Select;
        default:
            return std::optional<Gamepad::Button>();
    }
}

std::optional<Gamepad::Button> Application::convertSFMLJoystickButton(unsigned int button) const
{
    //Xbox-style button layout
    switch(button)
    {
        case 0u:
            return Gamepad::Button::A;
        case 1u:
            return Gamepad::Button::B;
        case 6u:
            return Gamepad::Button::Select;
        case 7u:
            return Gamepad::Button::Start;
        default:
            return std::optional<Gamepad::Button>();
    }
}
