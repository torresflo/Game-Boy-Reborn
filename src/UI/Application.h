#pragma once

#include <array>
#include <string>
#include <optional>

#include <SFML/Graphics.hpp>

#include "GameBoyEmulator.h"

#include "AudioRingBuffer.h"
#include "GameBoyAudioStream.h"

#include "CartridgeViewerWindow.h"
#include "RomFileDialog.h"
#include "SaveStateFileDialog.h"
#include "LoadStateFileDialog.h"
#include "ObjectViewerWindow.h"
#include "RegisterViewerWindow.h"
#include "TileDataViewerWindow.h"
#include "LogViewerWindow.h"
#include "DisassemblyWindow.h"
#include "MemoryHexViewerWindow.h"
#include "NotificationManager.h"

class Application
{
public:
    Application();
    ~Application();

    static Application& instance();

    void run();

    NotificationManager& getNotificationManager();

private:
    static constexpr double MaxAccumulatedFrames = 5.0;
    static constexpr float JoystickAxisThreshold = 50.f;
    static constexpr std::array<float, 8> SpeedPresets = {0.125f, 0.25f, 0.5f, 1.f, 1.5f, 2.f, 4.f, 8.f};

    void createWindow();
    void processEvents();
    void processKeyPressedEvent(const sf::Event::KeyPressed& key);
    void processKeyReleasedEvent(const sf::Event::KeyReleased& key);
    void updateGamepadInput();
    void applyJoystickAxisDirection(float axisPosition, Gamepad::Button negativeButton, Gamepad::Button positiveButton);
    void setSpeedMultiplier(float multiplier);
    void cycleSpeedPreset(int direction);
    void updateEmulation(sf::Time deltaTime);
    void update(sf::Time deltaTime);
    void render();
    void drawMenuBar();
    void updateWindowTitle();
    void updateGameScreenTransform(sf::Vector2u windowSize);

    std::optional<Gamepad::Button> convertSFMLKeyboardKey(const sf::Keyboard::Key& key) const;
    std::optional<Gamepad::Button> convertSFMLJoystickButton(unsigned int button) const;

    sf::RenderWindow window;
    sf::Texture gameScreenTexture;
    sf::Sprite gameScreenSprite;

    GameBoyEmulator emulator;

    AudioRingBuffer audioRingBuffer;
    GameBoyAudioStream audioStream{audioRingBuffer};

    RomFileDialog romFileDialog;
    SaveStateFileDialog saveStateFileDialog;
    LoadStateFileDialog loadStateFileDialog;
    RegisterViewerWindow registerViewerWindow;
    CartridgeViewerWindow cartridgeViewerWindow;
    TileDataViewerWindow tileDataViewerWindow;
    ObjectViewerWindow objectViewerWindow;
    LogViewerWindow logViewerWindow;
    DisassemblyWindow disassemblyWindow;
    MemoryHexViewerWindow memoryHexViewerWindow;
    NotificationManager notificationManager;

    static Application* instancePointer;

    std::array<bool, Gamepad::ButtonCount> keyboardButtonStates{};
    std::array<bool, Gamepad::ButtonCount> joystickButtonStates{};

    sf::Clock deltaClock;
    double frameTimeAccumulator = 0.0;
    float speedMultiplier = 1.0f;

    u32 windowWidth = 640u;
    u32 windowHeight = 576u;
    sf::State windowState = sf::State::Windowed;
    std::string romPathInWindowTitle;

    bool menuBarVisible = true;
    float menuBarHeight = 0.f;
};
