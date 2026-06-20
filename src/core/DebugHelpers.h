#pragma once

#include <iostream>
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

class Log
{
public:
    template<typename ...Args>
    static void print(LogLevel level, Args&& ... args);

    static void setLevel(LogLevel level);
    static bool isEnabled(LogLevel level);

private:
    static LogLevel currentLevel;
};

template <typename ...Args>
inline void Log::print(LogLevel level, Args &&...args)
{
    if(Log::isEnabled(level))
    {
        (std::cout << ... << args);
        std::cout << std::endl;
    }

    if(level == LogLevel::Fatal)
        exit(-1);
}

#define UNUSED(x) (void)(x);
