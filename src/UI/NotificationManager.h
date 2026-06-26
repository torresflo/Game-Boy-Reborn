#pragma once

#include <deque>
#include <string>

#include <SFML/Graphics.hpp>

#include "Common.h"

enum class NotificationLevel
{
    Info,
    Error
};

class NotificationManager
{
public:
    void push(NotificationLevel level, std::string message);

    void update(sf::Time deltaTime);
    void draw(const sf::RenderWindow& window, float topOffset) const;

private:
    struct ActiveNotification
    {
        NotificationLevel level;
        std::string message;
        float remainingSeconds;
        u64 id;
    };

    static constexpr float DisplayDurationSeconds = 3.5f;
    static constexpr float FadeOutDurationSeconds = 0.5f;
    static constexpr std::size_t MaxVisibleNotifications = 5;

    std::deque<ActiveNotification> activeNotifications;
    u64 nextId = 0;
};
