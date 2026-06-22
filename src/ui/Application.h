#pragma once

#include <string>

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
    void processEvents();
    void updateEmulation(sf::Time deltaTime);
    void update();
    void render();
    void drawMenuBar();
    void updateWindowTitle();
    void updateGameScreenTransform(sf::Vector2u windowSize);

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

    std::string romPathInWindowTitle;

    bool menuBarVisible = true;
    float menuBarHeight = 0.f;
};
