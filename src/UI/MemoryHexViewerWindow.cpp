#include "MemoryHexViewerWindow.h"
#include "GameBoyEmulator.h"
#include "MemoryBus.h"

MemoryHexViewerWindow::MemoryHexViewerWindow()
    : ToolWindow("Memory Viewer", WindowWidth, WindowHeight)
{
    memoryEditor.ReadOnly = true;
}

void MemoryHexViewerWindow::drawContent(GameBoyEmulator& emulator)
{
    ImGui::SetNextWindowPos({0.f, 0.f});
    ImGui::SetNextWindowSize({static_cast<float>(WindowWidth), static_cast<float>(WindowHeight)});
    ImGui::Begin("Memory Viewer", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    if(!emulator.isROMLoaded())
    {
        ImGui::TextDisabled("No ROM loaded.");
        ImGui::End();
        return;
    }

    emulator.getMemoryBus().dumpMemory(memoryBuffer);
    memoryEditor.DrawContents(memoryBuffer.data(), memoryBuffer.size());

    ImGui::End();
}
