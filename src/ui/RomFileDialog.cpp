#include "RomFileDialog.h"

#include <ImGuiFileDialog.h>

#include "Emulator.h"

void RomFileDialog::openDialog()
{
    ImGuiFileDialog::Instance()->OpenDialog(DialogKey, "Choose a ROM", ".gb,.gbc", IGFD::FileDialogConfig{});
}

void RomFileDialog::draw(Emulator& emulator)
{
    if(ImGuiFileDialog::Instance()->Display(DialogKey))
    {
        if(ImGuiFileDialog::Instance()->IsOk())
            emulator.loadROM(ImGuiFileDialog::Instance()->GetFilePathName());

        ImGuiFileDialog::Instance()->Close();
    }
}
