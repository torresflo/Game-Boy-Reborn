#include "RomFileDialog.h"

#include <imgui.h>
#include <ImGuiFileDialog.h>

#include "GameBoyEmulator.h"

RomFileDialog::RomFileDialog()
    : ToolWindow("Open ROM", WindowWidth, WindowHeight)
{
}

void RomFileDialog::onOpenRequested()
{
    createWindow(false);
    window->setVisible(true);
    open = true;

    ImGuiFileDialog::Instance()->OpenDialog(DialogKey, "Choose a ROM", ".gb,.gbc", IGFD::FileDialogConfig{});
}

void RomFileDialog::onClosed()
{
    // The window/context is created once and kept alive for the rest of the program (closing only
    // hides it - see above) instead of being destroyed and recreated on every use. ImGuiFileDialog
    // caches state tied to the ImGui context that was active the first time it drew (e.g. its
    // file-list ImGuiListClipper latches the context pointer it sees on its first Begin() call and
    // never updates it), so destroying that context and creating a new one for a later dialog
    // session would leave it pointing at freed memory.
    ImGuiFileDialog::Instance()->Close();
    window->setVisible(false);
    open = false;
}

void RomFileDialog::drawContent(GameBoyEmulator& emulator)
{
    const ImVec2 windowSize(static_cast<float>(WindowWidth), static_cast<float>(WindowHeight));
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));

    const bool dialogClosed = ImGuiFileDialog::Instance()->Display(DialogKey, ImGuiWindowFlags_NoCollapse, windowSize, windowSize);
    if(dialogClosed && ImGuiFileDialog::Instance()->IsOk())
        emulator.loadROM(ImGuiFileDialog::Instance()->GetFilePathName());

    // Deferred to the next update() call (see ToolWindow::update()), so this frame still renders
    // the dialog's last state while it's visible, before the window is hidden.
    if(dialogClosed)
        setOpen(false);
}
