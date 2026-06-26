#include "NotificationManager.h"

#include <algorithm>
#include <format>

#include <imgui.h>
#include <imgui-SFML.h>

namespace
{
    ImVec4 getColorForLevel(NotificationLevel level)
    {
        switch(level)
        {
            case NotificationLevel::Error:
                return ImVec4(1.f, 0.3f, 0.3f, 1.f);
            default:
                return ImVec4(1.f, 1.f, 1.f, 1.f);
        }
    }

    constexpr ImGuiWindowFlags NotificationWindowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
}

void NotificationManager::push(NotificationLevel level, std::string message)
{
    if(activeNotifications.size() >= MaxVisibleNotifications)
        activeNotifications.pop_front();

    activeNotifications.push_back({level, std::move(message), DisplayDurationSeconds, nextId++});
}

void NotificationManager::update(sf::Time deltaTime)
{
    for(ActiveNotification& notification : activeNotifications)
        notification.remainingSeconds -= deltaTime.asSeconds();

    std::erase_if(activeNotifications, [](const ActiveNotification& notification) { return notification.remainingSeconds <= 0.f; });
}

void NotificationManager::draw(const sf::RenderWindow& window, float topOffset) const
{
    if(activeNotifications.empty())
        return;

    ImGui::SFML::SetCurrentWindow(window);

    sf::Vector2u windowSize = window.getSize();
    constexpr float margin = 10.f;
    float cursorY = topOffset + margin;

    for(auto it = activeNotifications.rbegin(); it != activeNotifications.rend(); ++it)
    {
        float alpha = (it->remainingSeconds < FadeOutDurationSeconds) ? std::clamp(it->remainingSeconds / FadeOutDurationSeconds, 0.f, 1.f) : 1.f;

        ImGui::SetNextWindowPos(ImVec2(static_cast<float>(windowSize.x) - margin, cursorY), ImGuiCond_Always, ImVec2(1.f, 0.f));
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.85f * alpha);

        ImGui::Begin(std::format("##Notification{}", it->id).c_str(), nullptr, NotificationWindowFlags);

        ImVec4 color = getColorForLevel(it->level);
        color.w *= alpha;
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(it->message.c_str());
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

        cursorY += ImGui::GetWindowSize().y + margin;
        ImGui::End();
    }
}
