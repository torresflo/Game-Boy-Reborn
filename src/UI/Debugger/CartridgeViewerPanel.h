#pragma once

#include "DebugPanel.h"

class CartridgeViewerPanel : public DebugPanel
{
public:
    CartridgeViewerPanel();

    void draw(GameBoyEmulator& emulator) override;
};
