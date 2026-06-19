#include "Emulator.h"

#include <chrono>
#include <thread>

void Emulator::run(std::string path)
{
    if(!cartridge.loadROM(path))
        return;

    Log::print(LogLevel::Info, "ROM loaded successfully");

    bus.initialize(&cartridge, &CPU);
    CPU.initialize(&bus);

    running = true;
    paused = false;

    while(running)
    {
        if(paused)
        {
            delay(10);
            continue;
        }

        CPU.step();
    };
}

void Emulator::delay(u32 ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
