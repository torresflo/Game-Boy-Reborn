#pragma once

#include "FileDialogWindow.h"

class LoadStateFileDialog : public FileDialogWindow
{
public:
    LoadStateFileDialog();

protected:
    void onFileChosen(GameBoyEmulator& emulator, const std::string& filePath) override;
};
