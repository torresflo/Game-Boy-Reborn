#pragma once

#include <string>

#include <ImGuiFileDialog.h>

#include "ToolWindow.h"

// Base class for tool windows that host a single ImGuiFileDialog file picker (ROM loading,
// save state loading/saving, ...). Subclasses only need to supply the dialog parameters and
// override onFileChosen() with the action to run on the chosen file path.
//
// Each instance owns its own ImGuiFileDialog (rather than using ImGuiFileDialog::Instance()),
// because each FileDialogWindow runs in its own ImGuiContext (see ToolWindow::createWindow()).
// ImGuiFileDialog's internal ImGuiListClipper latches onto whichever ImGuiContext is active the
// first time it draws and never updates it, so sharing one ImGuiFileDialog across windows backed
// by different contexts corrupts that cached state and crashes.
class FileDialogWindow : public ToolWindow
{
public:
    FileDialogWindow(std::string windowTitle, unsigned int windowWidth, unsigned int windowHeight,
                      std::string fileDialogKey, std::string fileDialogTitle, std::string fileFilters,
                      IGFD::FileDialogConfig fileDialogConfig = {});

protected:
    void onOpenRequested() override;
    void onClosed() override;
    void drawContent(GameBoyEmulator& emulator) override;

    virtual void onFileChosen(GameBoyEmulator& emulator, const std::string& filePath) = 0;

private:
    std::string dialogKey;
    std::string dialogTitle;
    std::string fileFilters;
    IGFD::FileDialogConfig dialogConfig;
    ImGuiFileDialog fileDialog;
};
