#pragma once

#include "FileDialogWindow.h"

class SaveStateFileDialog : public FileDialogWindow
{
public:
    SaveStateFileDialog();

protected:
    void onFileChosen(GameBoyEmulator& emulator, const std::string& filePath) override;
};
