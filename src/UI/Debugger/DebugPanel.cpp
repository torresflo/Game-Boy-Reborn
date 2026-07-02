#include "DebugPanel.h"

DebugPanel::DebugPanel(std::string panelName)
    : name(std::move(panelName))
{
}

const std::string& DebugPanel::getName() const
{
    return name;
}
