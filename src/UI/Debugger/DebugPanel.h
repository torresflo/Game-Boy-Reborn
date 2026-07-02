#pragma once

#include <string>

class GameBoyEmulator;

class DebugPanel
{
public:
    explicit DebugPanel(std::string panelName);
    virtual ~DebugPanel() = default;

    const std::string& getName() const;

    virtual void draw(GameBoyEmulator& emulator) = 0;

protected:
    std::string name;
};
