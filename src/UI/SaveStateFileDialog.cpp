#include "SaveStateFileDialog.h"

#include <ImGuiFileDialog.h>

#include "GameBoyEmulator.h"

namespace
{
    constexpr unsigned int WindowWidth = 600;
    constexpr unsigned int WindowHeight = 400;

    IGFD::FileDialogConfig makeConfirmOverwriteConfig()
    {
        IGFD::FileDialogConfig config{};
        config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;
        return config;
    }
}

SaveStateFileDialog::SaveStateFileDialog()
    : FileDialogWindow("Save State", WindowWidth, WindowHeight, "SaveStateFileDialogKey", "Save State As", ".gbstate", makeConfirmOverwriteConfig())
{
}

void SaveStateFileDialog::onFileChosen(GameBoyEmulator& emulator, const std::string& filePath)
{
    emulator.saveState(filePath);
    emulator.setPaused(false);
}
