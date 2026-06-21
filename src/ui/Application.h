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
    void update();
    void render();
    void drawMenuBar();
    void updateWindowTitle();

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

    std::string romPathInWindowTitle;
};
