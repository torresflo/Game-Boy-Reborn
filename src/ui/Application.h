#pragma once

#include <SFML/Graphics.hpp>

#include "Emulator.h"

#include "CartridgeViewerWindow.h"
#include "RegisterViewerWindow.h"
#include "RomFileDialog.h"

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

    sf::RenderWindow window;
    sf::Texture gameScreenTexture;
    sf::Sprite gameScreenSprite;

    Emulator emulator;
    
    RomFileDialog romFileDialog;
    RegisterViewerWindow registerViewerWindow;
    CartridgeViewerWindow cartridgeViewerWindow;

    sf::Clock deltaClock;
};
