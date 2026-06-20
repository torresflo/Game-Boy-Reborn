#include "DebugHelpers.h"

#include <chrono>
#include <ctime>
#include <format>
#include <iostream>

LogLevel Log::currentLevel = LogLevel::All;
std::deque<LogEntry> Log::history;

void Log::setLevel(LogLevel level)
{
    Log::currentLevel = level;
}

LogLevel Log::getLevel()
{
    return Log::currentLevel;
}

bool Log::isEnabled(LogLevel level)
{
    return level >= Log::currentLevel;
}

const std::deque<LogEntry>& Log::getHistory()
{
    return history;
}

void Log::clearHistory()
{
    history.clear();
}

std::string Log::getCurrentTimestamp()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    auto milliseconds = duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm localTime{};
    localtime_s(&localTime, &time);

    return std::format("{:02}:{:02}:{:02}.{:03}", localTime.tm_hour, localTime.tm_min, localTime.tm_sec, milliseconds.count());
}
