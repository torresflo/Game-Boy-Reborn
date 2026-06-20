#include "LogViewerWindow.h"

#include <imgui.h>

#include "DebugHelpers.h"

namespace
{
    ImVec4 getColorForLogLevel(LogLevel level)
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

    constexpr const char* LogLevelNames[] = {"All", "Debug", "Info", "Warning", "Error", "Fatal", "None"};
}

LogViewerWindow::LogViewerWindow()
    : ToolWindow("Application Log", WindowWidth, WindowHeight)
{
}

void LogViewerWindow::drawContent(Emulator& emulator)
{
    UNUSED(emulator);

    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(WindowWidth), static_cast<float>(WindowHeight)));
    ImGui::Begin("Application Log", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    int selectedLevel = static_cast<int>(Log::getLevel());
    ImGui::SetNextItemWidth(100.f);
    if(ImGui::Combo("Level", &selectedLevel, LogLevelNames, IM_ARRAYSIZE(LogLevelNames)))
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

    ImGui::End();
}
