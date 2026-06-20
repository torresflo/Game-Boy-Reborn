#pragma once

#include "ToolWindow.h"

class RomFileDialog : public ToolWindow
{
public:
    RomFileDialog();

protected:
    void onOpenRequested() override;
    void onClosed() override;
    void drawContent(Emulator& emulator) override;

private:
    static constexpr const char* DialogKey = "RomFileDialogKey";
    static constexpr unsigned int WindowWidth = 600;
    static constexpr unsigned int WindowHeight = 400;
};
