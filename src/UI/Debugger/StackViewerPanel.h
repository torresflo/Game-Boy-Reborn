#pragma once

#include <imgui.h>

#include "DebugPanel.h"
#include "Common.h"

class StackViewerPanel : public DebugPanel
{
public:
    StackViewerPanel();

    void draw(GameBoyEmulator& emulator) override;

private:
    int stackWordCount = 32;

    static constexpr int MinimumStackWordCount = 1;
    static constexpr int MaximumStackWordCount = 256;

    static constexpr ImVec4 HeaderColor{0.55f, 0.75f, 1.f, 1.f};
    static constexpr ImVec4 AddressColor{0.60f, 0.60f, 0.65f, 1.f};
    static constexpr ImVec4 ValueColor{1.0f, 0.85f, 0.35f, 1.f};
    static constexpr ImVec4 StackPointerHighlightColor{0.95f, 0.50f, 0.10f, 0.35f};
};
