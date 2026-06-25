#pragma once

#include <array>
#include <string>
#include <optional>

#include <SFML/Graphics.hpp>

#include "GameBoyEmulator.h"

#include "AudioRingBuffer.h"
#include "GameBoyAudioStream.h"

#include "CartridgeViewerWindow.h"
#include "ObjectViewerWindow.h"
#include "RegisterViewerWindow.h"
#include "RomFileDialog.h"
#include "TileDataViewerWindow.h"
#include "LogViewerWindow.h"

class Application
{
public:
    Application();
    ~Application();

    void run();

private:
    void createWindow();
    void processEvents();
    void processKeyPressedEvent(const sf::Event::KeyPressed& key);
    void processKeyReleasedEvent(const sf::Event::KeyReleased& key);
    void updateGamepadInput();
    void applyJoystickAxisDirection(float axisPosition, Gamepad::Button negativeButton, Gamepad::Button positiveButton);
    void updateEmulation(sf::Time deltaTime);
    void update();
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
    RegisterViewerWindow registerViewerWindow;
    CartridgeViewerWindow cartridgeViewerWindow;
    TileDataViewerWindow tileDataViewerWindow;
    ObjectViewerWindow objectViewerWindow;
    LogViewerWindow logViewerWindow;

    std::array<bool, Gamepad::ButtonCount> keyboardButtonStates{};
    std::array<bool, Gamepad::ButtonCount> joystickButtonStates{};

    sf::Clock deltaClock;
    double frameTimeAccumulator = 0.0;

    u32 windowWidth = 640u;
    u32 windowHeight = 576u;
    sf::State windowState = sf::State::Windowed;
    std::string romPathInWindowTitle;

    bool menuBarVisible = true;
    float menuBarHeight = 0.f;
};
