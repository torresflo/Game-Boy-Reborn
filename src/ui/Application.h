#pragma once

#include <string>
#include <optional>

#include <SFML/Graphics.hpp>

#include "GameBoyEmulator.h"

#include "CartridgeViewerWindow.h"
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
    void updateEmulation(sf::Time deltaTime);
    void update();
    void render();
    void drawMenuBar();
    void updateWindowTitle();
    void updateGameScreenTransform(sf::Vector2u windowSize);

    std::optional<Gamepad::Button> convertSFMLKeyboardKey(const sf::Keyboard::Key& key) const;

    sf::RenderWindow window;
    sf::Texture gameScreenTexture;
    sf::Sprite gameScreenSprite;

    GameBoyEmulator emulator;
    
    RomFileDialog romFileDialog;
    RegisterViewerWindow registerViewerWindow;
    CartridgeViewerWindow cartridgeViewerWindow;
    TileDataViewerWindow tileDataViewerWindow;
    LogViewerWindow logViewerWindow;

    sf::Clock deltaClock;
    double frameTimeAccumulator = 0.0;

    u32 windowWidth = 640u;
    u32 windowHeight = 576u;
    sf::State windowState = sf::State::Windowed;
    std::string romPathInWindowTitle;

    bool menuBarVisible = true;
    float menuBarHeight = 0.f;
};
