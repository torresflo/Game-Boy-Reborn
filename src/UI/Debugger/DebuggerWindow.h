#pragma once

#include <imgui.h>

#include "ToolWindow.h"

#include "RegisterViewerPanel.h"
#include "StackViewerPanel.h"
#include "CartridgeViewerPanel.h"
#include "DisassemblyPanel.h"
#include "BreakpointPanel.h"
#include "TileDataViewerPanel.h"
#include "ObjectViewerPanel.h"
#include "BackgroundMapViewerPanel.h"
#include "ApuViewerPanel.h"
#include "MemoryHexViewerPanel.h"
#include "LogViewerPanel.h"

class DebugPanel;

class DebuggerWindow : public ToolWindow
{
public:
    DebuggerWindow();

protected:
    void onOpenRequested() override;
    void drawContent(GameBoyEmulator& emulator) override;

private:
    void buildDefaultDockLayout(ImGuiID dockspaceId);
    void drawDockedPanel(DebugPanel& panel, GameBoyEmulator& emulator);

    static constexpr unsigned int WindowWidth = 1860;
    static constexpr unsigned int WindowHeight = 960;

    RegisterViewerPanel registerViewerPanel;
    StackViewerPanel stackViewerPanel;
    CartridgeViewerPanel cartridgeViewerPanel;
    DisassemblyPanel disassemblyPanel;
    BreakpointPanel breakpointPanel;
    TileDataViewerPanel tileDataViewerPanel;
    ObjectViewerPanel objectViewerPanel;
    BackgroundMapViewerPanel backgroundMapViewerPanel;
    ApuViewerPanel apuViewerPanel;
    MemoryHexViewerPanel memoryHexViewerPanel;
    LogViewerPanel logViewerPanel;
};
