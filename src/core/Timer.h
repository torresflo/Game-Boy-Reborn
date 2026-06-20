#pragma once

#include "Common.h"

class CentralProcessingUnit;

class HardwareTimer
{
public:
    void initialize();
    void tick();

    void writeTimer(u16 address, u8 value);
    u8 readTimer(u16 address) const;

    void initialize(CentralProcessingUnit* cpuPtr);

private:
    u16 dividerRegister = 0; //DIV
    u8 timerCounter = 0; //TIMA
    u8 timerModulo = 0; //TMA
    u8 timerControl = 0; //TAC

    CentralProcessingUnit* CPU;
};