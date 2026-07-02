#pragma once

#include <array>

#include <imgui.h>

#include "DebugPanel.h"
#include "Common.h"

class LogViewerPanel : public DebugPanel
{
public:
    LogViewerPanel();

    void draw(GameBoyEmulator& emulator) override;

private:
    static ImVec4 getColorForLogLevel(LogLevel level);

    static constexpr std::array<const char*, 7> LogLevelNames = {"All", "Debug", "Info", "Warning", "Error", "Fatal", "None"};

    bool autoScroll = true;
};
