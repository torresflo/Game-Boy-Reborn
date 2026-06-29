#pragma once

#include <string>

#include <imgui.h>

#include "ToolWindow.h"

class RegisterViewerWindow : public ToolWindow
{
public:
    RegisterViewerWindow();

protected:
    void drawContent(GameBoyEmulator& emulator) override;

private:
    void drawSectionHeader(const char* label);
    bool beginRegisterTable(const char* id);
    void drawRegisterCell(const char* name, const std::string& valueText);
    void drawFlag(const char* label, bool set);
    void drawInfoLine(const char* name, const std::string& valueText);

    static constexpr unsigned int WindowWidth = 300;
    static constexpr unsigned int WindowHeight = 460;

    static constexpr ImVec4 HeaderColor{0.6f, 0.6f, 0.65f, 1.f};
    static constexpr ImVec4 NameColor{0.55f, 0.75f, 1.f, 1.f};
    static constexpr ImVec4 ValueColor{1.f, 0.85f, 0.35f, 1.f};
    static constexpr ImVec4 FlagSetColor{0.4f, 0.9f, 0.4f, 1.f};
    static constexpr ImVec4 FlagClearColor{0.5f, 0.5f, 0.5f, 1.f};

    static constexpr float RegisterNameColumnWidth = 45.f;
    static constexpr float RegisterValueColumnWidth = 65.f;
};
