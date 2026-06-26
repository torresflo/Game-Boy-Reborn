#pragma once

#include <string>

#include <ImGuiFileDialog.h>

#include "ToolWindow.h"

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
