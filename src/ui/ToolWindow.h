#pragma once

#include <optional>
#include <string>

#include <SFML/Graphics.hpp>

class Emulator;

class ToolWindow
{
public:
    ToolWindow(std::string title, unsigned int width, unsigned int height);
    virtual ~ToolWindow() = default;

    bool isOpen() const;
    void setOpen(bool open);

    void update(Emulator& emulator);

protected:
    void createWindow(bool startVisible = true);
    void destroyWindow();

    virtual void onOpenRequested();
    virtual void onClosed();

    virtual void drawContent(Emulator& emulator) = 0;

    std::optional<sf::RenderWindow> window;
    bool open = false;

private:
    void renderFrame(Emulator& emulator);

    std::string title;
    unsigned int width;
    unsigned int height;
    sf::Clock deltaClock;
    bool openRequested = false;
};
