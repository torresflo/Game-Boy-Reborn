#include "SaveStateFileDialog.h"

#include <filesystem>

#include <ImGuiFileDialog.h>

#include "Application.h"
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
    bool success = emulator.saveState(filePath);
    emulator.setPaused(false);

    std::string fileName = std::filesystem::path(filePath).filename().string();

    NotificationManager& notifications = Application::instance().getNotificationManager();
    if(success)
        notifications.push(NotificationLevel::Info, "State saved: " + fileName);
    else
        notifications.push(NotificationLevel::Error, "Failed to save state: " + fileName);
}
