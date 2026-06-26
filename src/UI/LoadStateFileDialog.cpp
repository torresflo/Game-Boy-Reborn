#include "LoadStateFileDialog.h"

#include <filesystem>

#include "Application.h"
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
    bool success = emulator.loadState(filePath);
    emulator.setPaused(false);

    std::string fileName = std::filesystem::path(filePath).filename().string();

    NotificationManager& notifications = Application::instance().getNotificationManager();
    if(success)
        notifications.push(NotificationLevel::Info, "State loaded: " + fileName);
    else
        notifications.push(NotificationLevel::Error, "Failed to load state: " + fileName);
}
