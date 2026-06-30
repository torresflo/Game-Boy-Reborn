#pragma once

#include <array>

#include <imgui.h>
#include "imgui_memory_editor.h"

#include "ToolWindow.h"
#include "TypeDefinitions.h"

class GameBoyEmulator;

class MemoryHexViewerWindow : public ToolWindow
{
public:
    MemoryHexViewerWindow();

protected:
    void drawContent(GameBoyEmulator& emulator) override;

private:
    static constexpr unsigned int WindowWidth  = 740;
    static constexpr unsigned int WindowHeight = 520;

    MemoryEditor memoryEditor;
    std::array<u8, 0x10000> memoryBuffer{};
};
