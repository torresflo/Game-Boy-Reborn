#pragma once

#include <imgui.h>

#include "DebugPanel.h"
#include "Common.h"

class BreakpointPanel : public DebugPanel
{
public:
    BreakpointPanel();

    void draw(GameBoyEmulator& emulator) override;

private:
    void drawExecutionControls(GameBoyEmulator& emulator);
    void drawBreakpointList(GameBoyEmulator& emulator);

    u16 breakpointInputAddress = 0x0000;

    static constexpr ImVec4 HeaderColor{0.55f, 0.75f, 1.f, 1.f};
    static constexpr ImVec4 AddressColor{0.60f, 0.60f, 0.65f, 1.f};
    static constexpr ImVec4 ValueColor{1.0f, 0.85f, 0.35f, 1.f};
    static constexpr ImVec4 BreakpointColor{0.90f, 0.25f, 0.25f, 1.f};
    static constexpr ImVec4 CurrentBreakpointHighlightColor{0.95f, 0.50f, 0.10f, 0.35f};
};
