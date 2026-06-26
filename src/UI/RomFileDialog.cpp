#include "RomFileDialog.h"

#include <filesystem>

#include "Application.h"
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
    bool success = emulator.loadROM(filePath);
    std::string fileName = std::filesystem::path(filePath).filename().string();

    NotificationManager& notifications = Application::instance().getNotificationManager();
    if(success)
        notifications.push(NotificationLevel::Info, "ROM loaded: " + fileName);
    else
        notifications.push(NotificationLevel::Error, "Failed to load ROM: " + fileName);
}
