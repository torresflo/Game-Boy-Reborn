#include "Emulator.h"

#include <chrono>
#include <thread>

#include "DebugHelpers.h"

EmulatorError Emulator::run(std::string path)
{
    if(!cartridge.loadROM(path))
        return EmulatorError::InvalidRomFile;

    Log::print(LogLevel::Info, "ROM loaded successfully");

    CPU.initialize();

    running = true;
    paused = false;
    cycles = 0;

    while(running)
    {
        if(paused)
        {
            delay(10);
            continue;
        }

        if(!CPU.step())
            return EmulatorError::CPUError;

        cycles++;
    };

    return EmulatorError::None;
}

void Emulator::delay(u32 ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
