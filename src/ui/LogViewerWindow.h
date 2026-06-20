#pragma once

#include "ToolWindow.h"

class LogViewerWindow : public ToolWindow
{
public:
    LogViewerWindow();

protected:
    void drawContent(Emulator& emulator) override;

private:
    static constexpr unsigned int WindowWidth = 600;
    static constexpr unsigned int WindowHeight = 400;

    bool autoScroll = true;
};
