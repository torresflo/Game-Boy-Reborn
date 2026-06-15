#include "CentralProcessingUnit.h"

#include "MathUtils.h"

u8 CentralProcessingUnit::noneInstruction()
{
    Log::print(LogLevel::Error, "None instruction called");
    exit(-1);
}

u8 CentralProcessingUnit::nopInstruction()
{
    return 0;
}

u8 CentralProcessingUnit::ldInstruction()
{
    u8 consumedCycles = 0;

    if(destinationIsMemory) //Example: LD (BC), A
    {
        if(currentInstruction.register2 >= RegisterType::AF) //16-bits register
        {
            memoryBus->write16(memoryDestination, fetchedData);
            consumedCycles++;
        }
        else
            memoryBus->write(memoryDestination, static_cast<u8>(fetchedData));
    }
    else if(currentInstruction.addressMode == AddressMode::HL_SPR)
    {
        u8 hFlag = (readRegister(currentInstruction.register2) & 0xF) + (fetchedData & 0xF) >= 0x10;
        u8 cFlag = (readRegister(currentInstruction.register2) & 0xFF) + (fetchedData & 0xFF) >= 0x100;
        setFlagValues(0, 0, hFlag, cFlag);
        u16 registerValue = readRegister(currentInstruction.register2) + static_cast<s8>(fetchedData);
        writeRegister(currentInstruction.register1, registerValue);
    }
    else
    {
        writeRegister(currentInstruction.register1, fetchedData);
    }

    return consumedCycles;
}

u8 CentralProcessingUnit::ldhInstruction()
{
    if(currentInstruction.register1 == RegisterType::A)
    {
        writeRegister(currentInstruction.register1, memoryBus->read(0xFF00 | fetchedData));
    }
    else
    {
        memoryBus->write(0xFF00 | fetchedData, registers.A);
    }

    return 1;
}

u8 CentralProcessingUnit::jpInstruction()
{
    if(checkCondition())
    {
        registers.PC = fetchedData;
        return 1;
    }
    return 0;
}

u8 CentralProcessingUnit::diInstruction()
{
    interruptMasterEnabled = false;

    return 0;
}

u8 CentralProcessingUnit::xorInstruction()
{
    registers.A ^= (fetchedData & 0xFF);
    setFlagValues(registers.A, 0, 0, 0);
    return 0;
}

bool CentralProcessingUnit::checkCondition() const
{
    switch(currentInstruction.condition)
    {
        case ConditionType::NONE:
            return true;
        case ConditionType::C:
            return flagC();
        case ConditionType::NC:
            return !flagC();
        case ConditionType::Z:
            return flagZ();
        case ConditionType::NZ:
            return !flagZ();
    }

    return false;
}

bool CentralProcessingUnit::flagZ() const
{
    return MathUtils<u8>::getBitValue(registers.F, 7);
}

bool CentralProcessingUnit::flagC() const
{
    return MathUtils<u8>::getBitValue(registers.F, 4);
}

void CentralProcessingUnit::setFlagValues(s8 z, s8 n, s8 h, s8 c)
{
    if(z != -1)
        MathUtils<u8>::setBitValue(registers.F, 7, z);

    if(n != -1)
        MathUtils<u8>::setBitValue(registers.F, 6, n);

    if(h != -1)
        MathUtils<u8>::setBitValue(registers.F, 5, h);

    if(c != -1)
        MathUtils<u8>::setBitValue(registers.F, 4, c);
}
