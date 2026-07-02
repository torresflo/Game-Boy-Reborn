#pragma once

#include <array>

#include <imgui.h>
#include "imgui_memory_editor.h"

#include "DebugPanel.h"
#include "TypeDefinitions.h"

class MemoryHexViewerPanel : public DebugPanel
{
public:
    MemoryHexViewerPanel();

    void draw(GameBoyEmulator& emulator) override;

private:
    MemoryEditor memoryEditor;
    std::array<u8, 0x10000> memoryBuffer{};
};
