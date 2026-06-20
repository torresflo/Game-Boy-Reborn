#pragma once

#include "ToolWindow.h"

class RegisterViewerWindow : public ToolWindow
{
public:
    RegisterViewerWindow();

protected:
    void drawContent(Emulator& emulator) override;

private:
    static constexpr unsigned int WindowWidth = 280;
    static constexpr unsigned int WindowHeight = 340;
};
