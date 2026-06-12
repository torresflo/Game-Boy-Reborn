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

private:
    static LogLevel currentLevel;
};

template <typename ...Args>
inline void Log::print(LogLevel level, Args &&...args)
{
    if(level >= Log::currentLevel)
    {
        (std::cout << ... << args);
        std::cout << std::endl;
    }
}

#define UNUSED(x) (void)(x)
