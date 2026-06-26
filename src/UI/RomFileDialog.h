#pragma once

#include "FileDialogWindow.h"

class RomFileDialog : public FileDialogWindow
{
public:
    RomFileDialog();

protected:
    void onFileChosen(GameBoyEmulator& emulator, const std::string& filePath) override;
};
