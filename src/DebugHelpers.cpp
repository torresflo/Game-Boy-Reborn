#include "DebugHelpers.h"

#include <iostream>

LogLevel Log::currentLevel = LogLevel::All;

void Log::setLevel(LogLevel level)
{
    Log::currentLevel = level;
}
