#pragma once

class Emulator;

class RomFileDialog
{
public:
    void openDialog();
    void draw(Emulator& emulator);

private:
    static constexpr const char* DialogKey = "RomFileDialogKey";
};
