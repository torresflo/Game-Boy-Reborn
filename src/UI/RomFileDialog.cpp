#include "RomFileDialog.h"

#include "GameBoyEmulator.h"

namespace
{
    constexpr unsigned int WindowWidth = 600;
    constexpr unsigned int WindowHeight = 400;
}

RomFileDialog::RomFileDialog()
    : FileDialogWindow("Open ROM", WindowWidth, WindowHeight, "RomFileDialogKey", "Choose a ROM", ".gb,.gbc")
{
}

void RomFileDialog::onFileChosen(GameBoyEmulator& emulator, const std::string& filePath)
{
    emulator.loadROM(filePath);
}
