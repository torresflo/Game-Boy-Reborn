#include "LoadStateFileDialog.h"

#include "GameBoyEmulator.h"

namespace
{
    constexpr unsigned int WindowWidth = 600;
    constexpr unsigned int WindowHeight = 400;
}

LoadStateFileDialog::LoadStateFileDialog()
    : FileDialogWindow("Load State", WindowWidth, WindowHeight, "LoadStateFileDialogKey", "Load State", ".gbstate")
{
}

void LoadStateFileDialog::onFileChosen(GameBoyEmulator& emulator, const std::string& filePath)
{
    emulator.loadState(filePath);
    emulator.setPaused(false);
}
