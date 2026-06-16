#include "CentralProcessingUnit.h"

#include "MathUtils.h"

const std::array<CentralProcessingUnit::InstructionFunc, static_cast<size_t>(InstructionType::COUNT)> CentralProcessingUnit::InstructionFuncs = []() {
    std::array<CentralProcessingUnit::InstructionFunc, static_cast<size_t>(InstructionType::COUNT)> arr{};
    arr[static_cast<size_t>(InstructionType::NONE)] = &CentralProcessingUnit::noneInstruction;
    arr[static_cast<size_t>(InstructionType::NOP)]  = &CentralProcessingUnit::nopInstruction;
    arr[static_cast<size_t>(InstructionType::LD)]   = &CentralProcessingUnit::ldInstruction;
    arr[static_cast<size_t>(InstructionType::LDH)]   = &CentralProcessingUnit::ldhInstruction;
    arr[static_cast<size_t>(InstructionType::JP)]   = &CentralProcessingUnit::jpInstruction;
    arr[static_cast<size_t>(InstructionType::JR)]   = &CentralProcessingUnit::jrInstruction;
    arr[static_cast<size_t>(InstructionType::CALL)]   = &CentralProcessingUnit::callInstruction;
    arr[static_cast<size_t>(InstructionType::RET)]   = &CentralProcessingUnit::retInstruction;
    arr[static_cast<size_t>(InstructionType::RETI)]   = &CentralProcessingUnit::retiInstruction;
    arr[static_cast<size_t>(InstructionType::RST)]   = &CentralProcessingUnit::rstInstruction;
    arr[static_cast<size_t>(InstructionType::DI)]   = &CentralProcessingUnit::diInstruction;
    arr[static_cast<size_t>(InstructionType::POP)]   = &CentralProcessingUnit::popInstruction;
    arr[static_cast<size_t>(InstructionType::PUSH)]   = &CentralProcessingUnit::pushInstruction;
    arr[static_cast<size_t>(InstructionType::XOR)]  = &CentralProcessingUnit::xorInstruction;
    return arr;
}();

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
    return gotoAddress(fetchedData, false);
}

u8 CentralProcessingUnit::jrInstruction()
{
    s8 relative = static_cast<s8>(fetchedData & 0xFF);
    u16 address = registers.PC + relative;
    return gotoAddress(address, false);
}

u8 CentralProcessingUnit::callInstruction()
{
    return gotoAddress(fetchedData, true);
}

u8 CentralProcessingUnit::retInstruction()
{
    u8 consumedCycles = 0;
    if(currentInstruction.condition != ConditionType::NONE)
        consumedCycles++;

    if(checkCondition())
    {
        u16 lo = stackPop();
        consumedCycles++;
        u16 hi = stackPop();
        consumedCycles++;

        u16 value = (hi << 8) | lo;
        registers.PC = value;
        consumedCycles++;
    }

    return consumedCycles;
}

u8 CentralProcessingUnit::retiInstruction()
{
    interruptMasterEnabled = true;
    return retInstruction();
}

u8 CentralProcessingUnit::rstInstruction()
{
    return gotoAddress(currentInstruction.param, true);
}

u8 CentralProcessingUnit::popInstruction()
{
    u8 consumedCycles = 0;
    u16 lo = stackPop();
    consumedCycles++;
    u16 hi = stackPop();
    consumedCycles++;

    u16 value = (hi << 8) | lo;
    if(currentInstruction.register1 == RegisterType::AF)
        writeRegister(currentInstruction.register1, value & 0xFFF0);
    else
        writeRegister(currentInstruction.register1, value);

    return consumedCycles;
}

u8 CentralProcessingUnit::pushInstruction()
{
    u8 consumedCycles = 0;

    u16 hi = (readRegister(currentInstruction.register1) >> 8) & 0xFF;
    consumedCycles++;
    stackPush(static_cast<u8>(hi));

    u16 lo = readRegister(currentInstruction.register1) & 0xFF;
    consumedCycles++;
    stackPush(static_cast<u8>(lo));

    consumedCycles++;
    return consumedCycles;
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

u8 CentralProcessingUnit::gotoAddress(u16 address, bool pushPC)
{
    u8 consumedCycles = 0;
    if(checkCondition())
    {
        if(pushPC)
        {
            stackPush16(registers.PC);
            consumedCycles += 2;
        }

        registers.PC = address;
        consumedCycles++;
    }
    return consumedCycles;
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
