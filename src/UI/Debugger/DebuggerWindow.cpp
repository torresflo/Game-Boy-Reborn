#include "DebuggerWindow.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "DebugPanel.h"

DebuggerWindow::DebuggerWindow()
    : ToolWindow("Debugger", WindowWidth, WindowHeight, sf::Style::Default)
{
}

void DebuggerWindow::onOpenRequested()
{
    ToolWindow::onOpenRequested();

    ::ShowWindow(window->getNativeHandle(), SW_MAXIMIZE);

    // A fresh ImGui-SFML context is created every time this window (re)opens (see ToolWindow),
    // so docking has to be re-enabled and the ini path re-pointed each time too.
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = "DebuggerLayout.ini";
}

void DebuggerWindow::buildDefaultDockLayout(ImGuiID dockspaceId)
{
    ImGui::DockBuilderRemoveNode(dockspaceId);
    ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->WorkSize);

    ImGuiID leftGroup = 0, rightColumn = 0;
    ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, 2.f / 3.f, &leftGroup, &rightColumn);

    ImGuiID leftColumn = 0, centerColumn = 0;
    ImGui::DockBuilderSplitNode(leftGroup, ImGuiDir_Left, 1.f / 2.f, &leftColumn, &centerColumn);

    ImGuiID leftColumnTop = 0, logNode = 0;
    ImGui::DockBuilderSplitNode(leftColumn, ImGuiDir_Up, 0.8f, &leftColumnTop, &logNode);

    ImGuiID cartridgeNode = 0, registersDisassemblyGroup = 0;
    ImGui::DockBuilderSplitNode(leftColumnTop, ImGuiDir_Up, 0.2f, &cartridgeNode, &registersDisassemblyGroup);

    ImGuiID registersNode = 0, disassemblyNode = 0;
    ImGui::DockBuilderSplitNode(registersDisassemblyGroup, ImGuiDir_Left, 0.5f, &registersNode, &disassemblyNode);

    ImGuiID tileObjectsGroup = 0, backgroundMapNode = 0;
    ImGui::DockBuilderSplitNode(centerColumn, ImGuiDir_Up, 0.42f, &tileObjectsGroup, &backgroundMapNode);

    ImGuiID tileDataNode = 0, objectsNode = 0;
    ImGui::DockBuilderSplitNode(tileObjectsGroup, ImGuiDir_Left, 0.5f, &tileDataNode, &objectsNode);

    ImGuiID apuNode = 0, memoryNode = 0;
    ImGui::DockBuilderSplitNode(rightColumn, ImGuiDir_Up, 0.7f, &apuNode, &memoryNode);

    ImGui::DockBuilderDockWindow(cartridgeViewerPanel.getName().c_str(), cartridgeNode);
    ImGui::DockBuilderDockWindow(registerViewerPanel.getName().c_str(), registersNode);
    ImGui::DockBuilderDockWindow(stackViewerPanel.getName().c_str(), registersNode);
    ImGui::DockBuilderDockWindow(disassemblyPanel.getName().c_str(), disassemblyNode);
    ImGui::DockBuilderDockWindow(breakpointPanel.getName().c_str(), disassemblyNode);
    ImGui::DockBuilderDockWindow(logViewerPanel.getName().c_str(), logNode);
    ImGui::DockBuilderDockWindow(tileDataViewerPanel.getName().c_str(), tileDataNode);
    ImGui::DockBuilderDockWindow(objectViewerPanel.getName().c_str(), objectsNode);
    ImGui::DockBuilderDockWindow(backgroundMapViewerPanel.getName().c_str(), backgroundMapNode);
    ImGui::DockBuilderDockWindow(apuViewerPanel.getName().c_str(), apuNode);
    ImGui::DockBuilderDockWindow(memoryHexViewerPanel.getName().c_str(), memoryNode);

    ImGui::DockBuilderFinish(dockspaceId);
}

void DebuggerWindow::drawDockedPanel(DebugPanel& panel, GameBoyEmulator& emulator)
{
    ImGui::Begin(panel.getName().c_str(), nullptr, ImGuiWindowFlags_HorizontalScrollbar);
    panel.draw(emulator);
    ImGui::End();
}

void DebuggerWindow::drawContent(GameBoyEmulator& emulator)
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    constexpr ImGuiWindowFlags HostWindowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
    ImGui::Begin("DebuggerDockHost", nullptr, HostWindowFlags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspaceId = ImGui::GetID("DebuggerDockSpace");
    if(ImGui::DockBuilderGetNode(dockspaceId) == nullptr)
        buildDefaultDockLayout(dockspaceId);

    ImGui::DockSpace(dockspaceId);

    ImGui::End();

    drawDockedPanel(registerViewerPanel, emulator);
    drawDockedPanel(stackViewerPanel, emulator);
    drawDockedPanel(cartridgeViewerPanel, emulator);
    drawDockedPanel(disassemblyPanel, emulator);
    drawDockedPanel(breakpointPanel, emulator);
    drawDockedPanel(tileDataViewerPanel, emulator);
    drawDockedPanel(objectViewerPanel, emulator);
    drawDockedPanel(backgroundMapViewerPanel, emulator);
    drawDockedPanel(apuViewerPanel, emulator);
    drawDockedPanel(memoryHexViewerPanel, emulator);
    drawDockedPanel(logViewerPanel, emulator);
}
