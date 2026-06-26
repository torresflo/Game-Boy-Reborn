#pragma once

#include <deque>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdio>
#include <cstdarg>

enum class LogLevel
{
    All = 0,
    Debug,
    Info,
    Warning,
    Error,
    Fatal,
    None
};

struct LogEntry
{
    LogLevel level;
    std::string timestamp;
    std::string message;
};

class Log
{
public:
    template<typename ...Args>
    static void print(LogLevel level, Args&& ... args);

    static void setLevel(LogLevel level);
    static LogLevel getLevel();
    static bool isEnabled(LogLevel level);

    static const std::deque<LogEntry>& getHistory();
    static void clearHistory();

private:
    static std::string getCurrentTimestamp();

    static LogLevel currentLevel;
    static std::deque<LogEntry> history;
    static constexpr std::size_t MaxHistorySize = 1000;
};

template <typename ...Args>
inline void Log::print(LogLevel level, Args &&...args)
{
    if(Log::isEnabled(level))
    {
        std::ostringstream stream;
        (stream << ... << args);
        std::string message = stream.str();

        std::cout << message << std::endl;

        history.push_back(LogEntry{level, getCurrentTimestamp(), std::move(message)});
        if(history.size() > MaxHistorySize)
            history.pop_front();
    }

    if(level == LogLevel::Fatal)
        exit(-1);
}

#define UNUSED(x) (void)(x);
