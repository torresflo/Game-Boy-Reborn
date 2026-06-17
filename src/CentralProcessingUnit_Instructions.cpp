#include "CentralProcessingUnit.h"

#include "MathUtils.h"

const std::array<CentralProcessingUnit::InstructionFunc, static_cast<size_t>(InstructionType::COUNT)> CentralProcessingUnit::InstructionFuncs = []() {
    std::array<CentralProcessingUnit::InstructionFunc, static_cast<size_t>(InstructionType::COUNT)> arr{};
    arr[static_cast<size_t>(InstructionType::NONE)] = &CentralProcessingUnit::noneInstruction;
    arr[static_cast<size_t>(InstructionType::NOP)]  = &CentralProcessingUnit::nopInstruction;
    arr[static_cast<size_t>(InstructionType::LD)]   = &CentralProcessingUnit::ldInstruction;
    arr[static_cast<size_t>(InstructionType::LDH)]  = &CentralProcessingUnit::ldhInstruction;
    arr[static_cast<size_t>(InstructionType::JP)]   = &CentralProcessingUnit::jpInstruction;
    arr[static_cast<size_t>(InstructionType::JR)]   = &CentralProcessingUnit::jrInstruction;
    arr[static_cast<size_t>(InstructionType::CALL)] = &CentralProcessingUnit::callInstruction;
    arr[static_cast<size_t>(InstructionType::RET)]  = &CentralProcessingUnit::retInstruction;
    arr[static_cast<size_t>(InstructionType::RETI)] = &CentralProcessingUnit::retiInstruction;
    arr[static_cast<size_t>(InstructionType::RST)]  = &CentralProcessingUnit::rstInstruction;
    arr[static_cast<size_t>(InstructionType::INC)]  = &CentralProcessingUnit::incInstruction;
    arr[static_cast<size_t>(InstructionType::ADD)]  = &CentralProcessingUnit::addInstruction;
    arr[static_cast<size_t>(InstructionType::ADC)]  = &CentralProcessingUnit::adcInstruction;
    arr[static_cast<size_t>(InstructionType::SUB)]  = &CentralProcessingUnit::subInstruction;
    arr[static_cast<size_t>(InstructionType::SBC)]  = &CentralProcessingUnit::sbcInstruction;
    arr[static_cast<size_t>(InstructionType::DEC)]  = &CentralProcessingUnit::decInstruction;
    arr[static_cast<size_t>(InstructionType::DI)]   = &CentralProcessingUnit::diInstruction;
    arr[static_cast<size_t>(InstructionType::POP)]  = &CentralProcessingUnit::popInstruction;
    arr[static_cast<size_t>(InstructionType::PUSH)] = &CentralProcessingUnit::pushInstruction;
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
        if(is16BitsRegister(currentInstruction.register2))
        {
            memoryBus->write16(memoryDestination, fetchedData);
            consumedCycles++;
        }
        else
            memoryBus->write(memoryDestination, static_cast<u8>(fetchedData));

        consumedCycles++;
    }
    else if(currentInstruction.addressMode == AddressMode::HL_SPR)
    {
        s8 hFlag = (readRegister(currentInstruction.register2) & 0xF) + (fetchedData & 0xF) >= 0x10;
        s8 cFlag = (readRegister(currentInstruction.register2) & 0xFF) + (fetchedData & 0xFF) >= 0x100;
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
        memoryBus->write(memoryDestination, registers.A);
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

    writeRegister(currentInstruction.register1, value);

    if(currentInstruction.register1 == RegisterType::AF)
        writeRegister(currentInstruction.register1, value & 0xFFF0);

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

u8 CentralProcessingUnit::incInstruction()
{
    u8 consumedCycles = 0;
    u16 value = readRegister(currentInstruction.register1) + 1;

    if(is16BitsRegister(currentInstruction.register1))
        consumedCycles++;

    if(currentInstruction.register1 == RegisterType::HL
        && currentInstruction.addressMode == AddressMode::MR)
    {  
        u16 address = readRegister(RegisterType::HL);
        value = memoryBus->read(address) + 1;
        value &= 0xFF;
        memoryBus->write(address, static_cast<u8>(value));
    }
    else
    {
        writeRegister(currentInstruction.register1, value);
        value = readRegister(currentInstruction.register1);
    }

    if((currentOPCode & 0x03) == 0x03)
        return consumedCycles;

    setFlagValues(value == 0, 0, (value & 0x0F) == 0, -1);
    return consumedCycles;
}

u8 CentralProcessingUnit::decInstruction()
{
    u8 consumedCycles = 0;
    u16 value = readRegister(currentInstruction.register1) - 1;

    if(is16BitsRegister(currentInstruction.register1))
        consumedCycles++;

    if(currentInstruction.register1 == RegisterType::HL
        && currentInstruction.addressMode == AddressMode::MR)
    {  
        u16 address = readRegister(RegisterType::HL);
        value = memoryBus->read(address) - 1;
        memoryBus->write(address, static_cast<u8>(value));
    }
    else
    {
        writeRegister(currentInstruction.register1, value);
        value = readRegister(currentInstruction.register1);
    }

    if((currentOPCode & 0x0B) == 0x0B)
        return consumedCycles;

    setFlagValues(value == 0, 1, (value & 0x0F) == 0x0F, -1);
    return consumedCycles;
}

u8 CentralProcessingUnit::addInstruction()
{
    u8 consumedCycles = 0;
    u32 value = readRegister(currentInstruction.register1) + fetchedData;
    bool isUsing16BitsRegister = is16BitsRegister(currentInstruction.register1);
    
    if(isUsing16BitsRegister)
    {
        consumedCycles++;
    }

    if(currentInstruction.register1 == RegisterType::SP)
    {
        value = readRegister(currentInstruction.register1) + static_cast<s8>(fetchedData); //Can be negative to perform a substract
    }

    //Compute flags
    s8 zFlag = 0, hFlag = 0, cFlag = 0;
    if(isUsing16BitsRegister && currentInstruction.register1 != RegisterType::SP)
    {
        zFlag = -1;
        hFlag = (readRegister(currentInstruction.register1) & 0xFFF) + (fetchedData & 0xFFF) >= 0x1000;
        cFlag = static_cast<u32>(readRegister(currentInstruction.register1)) + static_cast<s32>(fetchedData) >= 0x10000;
    }
    else
    {
        if(currentInstruction.register1 == RegisterType::SP)
            zFlag = 0;
        else
            zFlag = (value & 0xFF) == 0;

        hFlag = (readRegister(currentInstruction.register1) & 0xF) + (fetchedData & 0xF) >= 0x10;
        cFlag = static_cast<s32>(readRegister(currentInstruction.register1) & 0xFF) + static_cast<s32>(fetchedData & 0xFF) >= 0x100;
    }

    writeRegister(currentInstruction.register1, value & 0xFFFF);
    setFlagValues(zFlag, 0, hFlag, cFlag);

    return consumedCycles;
}

u8 CentralProcessingUnit::adcInstruction()
{
    u16 data = fetchedData;
    u16 registerA = readRegister(RegisterType::A);
    u16 cFlag = flagC();

    writeRegister(RegisterType::A, (registerA + data + cFlag) & 0xFF);
    setFlagValues(readRegister(RegisterType::A) == 0, 0, (registerA & 0xF) + (data & 0xF) + cFlag > 0xF, registerA + data + cFlag > 0xFF);
    return 0;
}

u8 CentralProcessingUnit::subInstruction()
{
    u16 value = readRegister(currentInstruction.register1) - fetchedData;
    bool zFlag = value == 0;
    bool hFlag = (static_cast<s32>(readRegister(currentInstruction.register1)) & 0xF) - (static_cast<s32>(fetchedData) & 0xF) < 0;
    bool cFlag = static_cast<s32>(readRegister(currentInstruction.register1)) - static_cast<s32>(fetchedData) < 0;

    writeRegister(currentInstruction.register1, value);
    setFlagValues(zFlag, 1, hFlag, cFlag);
    return 0;
}

u8 CentralProcessingUnit::sbcInstruction()
{
    u8 value = static_cast<u8>(fetchedData + flagC());
    u16 registerValue = readRegister(currentInstruction.register1) - value;
    bool zFlag = registerValue == 0;
    bool hFlag = (static_cast<s32>(readRegister(currentInstruction.register1)) & 0xF) - (static_cast<s32>(fetchedData) & 0xF) - static_cast<s32>(flagC()) < 0;
    bool cFlag = static_cast<s32>(readRegister(currentInstruction.register1)) - static_cast<s32>(fetchedData) - static_cast<s32>(flagC()) < 0;

    writeRegister(currentInstruction.register1, registerValue);
    setFlagValues(zFlag, 1, hFlag, cFlag);
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
    setFlagValues(registers.A == 0, 0, 0, 0);
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

bool CentralProcessingUnit::flagN() const
{
    return MathUtils<u8>::getBitValue(registers.F, 6);
}

bool CentralProcessingUnit::flagH() const
{
    return MathUtils<u8>::getBitValue(registers.F, 5);
}

bool CentralProcessingUnit::flagC() const
{
    return MathUtils<u8>::getBitValue(registers.F, 4);
}

void CentralProcessingUnit::setFlagValues(s8 zFlag, s8 nFlag, s8 hFlag, s8 cFlag)
{
    if(zFlag != -1)
        MathUtils<u8>::setBitValue(registers.F, 7, zFlag);

    if(nFlag != -1)
        MathUtils<u8>::setBitValue(registers.F, 6, nFlag);

    if(hFlag != -1)
        MathUtils<u8>::setBitValue(registers.F, 5, hFlag);

    if(cFlag != -1)
        MathUtils<u8>::setBitValue(registers.F, 4, cFlag);
}
