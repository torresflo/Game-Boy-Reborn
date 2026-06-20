#pragma once

#include "ToolWindow.h"

class CartridgeViewerWindow : public ToolWindow
{
public:
    CartridgeViewerWindow();

protected:
    void drawContent(Emulator& emulator) override;

private:
    static constexpr unsigned int WindowWidth = 320;
    static constexpr unsigned int WindowHeight = 280;
};
