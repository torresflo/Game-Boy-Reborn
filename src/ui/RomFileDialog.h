#pragma once

#include <optional>

#include <SFML/Graphics.hpp>

class Emulator;

class RomFileDialog
{
public:
    void openDialog();
    void update(Emulator& emulator);

private:
    void ensureWindowCreated();

    static constexpr const char* DialogKey = "RomFileDialogKey";
    static constexpr unsigned int WindowWidth = 600;
    static constexpr unsigned int WindowHeight = 400;

    std::optional<sf::RenderWindow> window;
    sf::Clock deltaClock;
    bool pendingOpen = false;
    bool open = false;
};
