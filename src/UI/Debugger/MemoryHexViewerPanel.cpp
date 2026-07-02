#include "MemoryHexViewerPanel.h"
#include "GameBoyEmulator.h"
#include "MemoryBus.h"

MemoryHexViewerPanel::MemoryHexViewerPanel()
    : DebugPanel("Memory Viewer")
{
    memoryEditor.ReadOnly = true;
}

void MemoryHexViewerPanel::draw(GameBoyEmulator& emulator)
{
    if(!emulator.isROMLoaded())
    {
        ImGui::TextDisabled("No ROM loaded.");
        return;
    }

    emulator.getMemoryBus().dumpMemory(memoryBuffer);
    memoryEditor.DrawContents(memoryBuffer.data(), memoryBuffer.size());
}
