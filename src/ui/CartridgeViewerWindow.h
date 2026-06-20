#pragma once

#include <optional>

#include <SFML/Graphics.hpp>

class Cartridge;

class CartridgeViewerWindow
{
public:
    bool isOpen() const;
    void setOpen(bool open);

    void update(const Cartridge& cartridge);

private:
    void createWindow();
    void closeWindow();

    static constexpr unsigned int WindowWidth = 320;
    static constexpr unsigned int WindowHeight = 280;

    std::optional<sf::RenderWindow> window;
    sf::Clock deltaClock;
    bool open = false;
};
