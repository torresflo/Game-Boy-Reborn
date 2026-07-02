#include "LogViewerPanel.h"

#include <imgui.h>

LogViewerPanel::LogViewerPanel()
    : DebugPanel("Application Log")
{
}

ImVec4 LogViewerPanel::getColorForLogLevel(LogLevel level)
{
    switch(level)
    {
        case LogLevel::Debug:
            return ImVec4(0.6f, 0.6f, 0.6f, 1.f);
        case LogLevel::Warning:
            return ImVec4(1.f, 0.8f, 0.2f, 1.f);
        case LogLevel::Error:
        case LogLevel::Fatal:
            return ImVec4(1.f, 0.3f, 0.3f, 1.f);
        default:
            return ImVec4(1.f, 1.f, 1.f, 1.f);
    }
}

void LogViewerPanel::draw(GameBoyEmulator& emulator)
{
    UNUSED(emulator);

    int selectedLevel = static_cast<int>(Log::getLevel());
    ImGui::SetNextItemWidth(100.f);
    if(ImGui::Combo("Level", &selectedLevel, LogLevelNames.data(), static_cast<int>(LogLevelNames.size())))
        Log::setLevel(static_cast<LogLevel>(selectedLevel));

    ImGui::SameLine();
    if(ImGui::Button("Clear"))
        Log::clearHistory();

    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &autoScroll);

    ImGui::Separator();

    ImGui::BeginChild("LogScrollRegion", ImVec2(0.f, 0.f), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);

    for(const LogEntry& entry : Log::getHistory())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, getColorForLogLevel(entry.level));
        ImGui::Text("[%s] %s", entry.timestamp.c_str(), entry.message.c_str());
        ImGui::PopStyleColor();
    }

    if(autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.f);

    ImGui::EndChild();
}
