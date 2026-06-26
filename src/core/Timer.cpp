#include "Timer.h"

#include <format>

#include "CentralProcessingUnit.h"
#include "MathUtils.h"
#include "save/SaveStateReader.h"
#include "save/SaveStateWriter.h"

void HardwareTimer::initialize()
{
    dividerRegister = 0xAC00;
}

void HardwareTimer::tick()
{
    u16 previousDiv = dividerRegister;
    dividerRegister++;

    bool timerUpdate = false;
    switch(timerControl & 0b11)
    {
        case 0b00:
            timerUpdate = MathUtils<u16>::getBitValue(previousDiv, 9) && !MathUtils<u16>::getBitValue(dividerRegister, 9);
            break;
        case 0b01:
            timerUpdate = MathUtils<u16>::getBitValue(previousDiv, 3) && !MathUtils<u16>::getBitValue(dividerRegister, 3);
            break;
        case 0b10:
            timerUpdate = MathUtils<u16>::getBitValue(previousDiv, 5) && !MathUtils<u16>::getBitValue(dividerRegister, 5);
            break;
        case 0b11:
            timerUpdate = MathUtils<u16>::getBitValue(previousDiv, 7) && !MathUtils<u16>::getBitValue(dividerRegister, 7);
            break;
    }

    if(timerUpdate && MathUtils<u8>::getBitValue(timerControl, 2))
    {
        timerCounter++;
        if(timerCounter == 0xFF)
        {
            timerCounter = timerModulo;
            CPU->requestInterrupt(InterruptType::Timer);
        }
    }
}

void HardwareTimer::writeTimer(u16 address, u8 value)
{
    switch(address)
    {
        case 0xFF04:
            dividerRegister = 0;
            break;
        case 0xFF05:
            timerCounter = value;
            break;
        case 0xFF06:
            timerModulo = value;
            break;
        case 0xFF07:
            timerControl = value;
            break;
    }
}

u8 HardwareTimer::readTimer(u16 address) const
{
    switch(address)
    {
        case 0xFF04:
            return dividerRegister >> 8;
        case 0xFF05:
            return timerCounter;
        case 0xFF06:
            return timerModulo;
        case 0xFF07:
            return timerControl;
    }

    Log::print(LogLevel::Error, std::format("Unsupported timer reading (0x{:4X}).", address));
    return 0;
}

void HardwareTimer::initialize(CentralProcessingUnit* cpuPtr)
{
    this->CPU = cpuPtr;

    dividerRegister = 0xABCC;
}

void HardwareTimer::serialize(SaveStateWriter& writer) const
{
    writer.write(dividerRegister);
    writer.write(timerCounter);
    writer.write(timerModulo);
    writer.write(timerControl);
}

void HardwareTimer::deserialize(SaveStateReader& reader)
{
    reader.read(dividerRegister);
    reader.read(timerCounter);
    reader.read(timerModulo);
    reader.read(timerControl);
}
