#pragma once

#include <optional>
#include <string>

#include <SFML/Graphics.hpp>

#include "TypeDefinitions.h"

class GameBoyEmulator;

class ToolWindow
{
public:
    ToolWindow(std::string title, unsigned int width, unsigned int height, u32 style = sf::Style::Titlebar | sf::Style::Close);
    virtual ~ToolWindow() = default;

    bool isOpen() const;
    void setOpen(bool open);

    void update(GameBoyEmulator& emulator);

protected:
    void createWindow(bool startVisible = true);
    void destroyWindow();

    virtual void onOpenRequested();
    virtual void onClosed();

    virtual void drawContent(GameBoyEmulator& emulator) = 0;

    std::optional<sf::RenderWindow> window;
    bool open = false;

private:
    void renderFrame(GameBoyEmulator& emulator);

    std::string title;
    unsigned int width;
    unsigned int height;
    u32 windowStyle;
    sf::Clock deltaClock;
    bool openRequested = false;
};
