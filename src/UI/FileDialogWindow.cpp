#include "FileDialogWindow.h"

#include <imgui.h>

#include "GameBoyEmulator.h"

FileDialogWindow::FileDialogWindow(std::string windowTitle, unsigned int windowWidth, unsigned int windowHeight,
                                    std::string fileDialogKey, std::string fileDialogTitle, std::string fileDialogFilters,
                                    IGFD::FileDialogConfig fileDialogConfig)
    : ToolWindow(std::move(windowTitle), windowWidth, windowHeight), dialogKey(std::move(fileDialogKey)),
      dialogTitle(std::move(fileDialogTitle)), fileFilters(std::move(fileDialogFilters)), dialogConfig(std::move(fileDialogConfig))
{
}

void FileDialogWindow::onOpenRequested()
{
    createWindow(false);
    window->setVisible(true);
    open = true;

    fileDialog.OpenDialog(dialogKey, dialogTitle, fileFilters.c_str(), dialogConfig);
}

void FileDialogWindow::onClosed()
{
    // The window/context is created once and kept alive for the rest of the program (closing only
    // hides it - see above) instead of being destroyed and recreated on every use. ImGuiFileDialog
    // caches state tied to the ImGui context that was active the first time it drew (e.g. its
    // file-list ImGuiListClipper latches the context pointer it sees on its first Begin() call and
    // never updates it), so destroying that context and creating a new one for a later dialog
    // session would leave it pointing at freed memory.
    fileDialog.Close();
    window->setVisible(false);
    open = false;
}

void FileDialogWindow::drawContent(GameBoyEmulator& emulator)
{
    const ImVec2 windowSize(static_cast<float>(window->getSize().x), static_cast<float>(window->getSize().y));
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));

    const bool dialogClosed = fileDialog.Display(dialogKey, ImGuiWindowFlags_NoCollapse, windowSize, windowSize);
    if(dialogClosed && fileDialog.IsOk())
        onFileChosen(emulator, fileDialog.GetFilePathName());

    // Deferred to the next update() call (see ToolWindow::update()), so this frame still renders
    // the dialog's last state while it's visible, before the window is hidden.
    if(dialogClosed)
        setOpen(false);
}
