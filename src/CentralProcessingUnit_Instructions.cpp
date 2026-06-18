#include "CentralProcessingUnit.h"

#include "MathUtils.h"

const std::array<CentralProcessingUnit::InstructionFunc, static_cast<size_t>(InstructionType::COUNT)> CentralProcessingUnit::InstructionFuncs = []() {
    std::array<CentralProcessingUnit::InstructionFunc, static_cast<size_t>(InstructionType::COUNT)> arr{};
    arr[static_cast<size_t>(InstructionType::NONE)] = &CentralProcessingUnit::noneInstruction;
    arr[static_cast<size_t>(InstructionType::NOP)]  = &CentralProcessingUnit::nopInstruction;
    arr[static_cast<size_t>(InstructionType::AND)]  = &CentralProcessingUnit::andInstruction;
    arr[static_cast<size_t>(InstructionType::OR)]  = &CentralProcessingUnit::orInstruction;
    arr[static_cast<size_t>(InstructionType::XOR)]  = &CentralProcessingUnit::xorInstruction;
    arr[static_cast<size_t>(InstructionType::CP)]  = &CentralProcessingUnit::cpInstruction;
    arr[static_cast<size_t>(InstructionType::LD)]   = &CentralProcessingUnit::ldInstruction;
    arr[static_cast<size_t>(InstructionType::LDH)]  = &CentralProcessingUnit::ldhInstruction;
    arr[static_cast<size_t>(InstructionType::JP)]   = &CentralProcessingUnit::jpInstruction;
    arr[static_cast<size_t>(InstructionType::JR)]   = &CentralProcessingUnit::jrInstruction;
    arr[static_cast<size_t>(InstructionType::CALL)] = &CentralProcessingUnit::callInstruction;
    arr[static_cast<size_t>(InstructionType::RET)]  = &CentralProcessingUnit::retInstruction;
    arr[static_cast<size_t>(InstructionType::RETI)] = &CentralProcessingUnit::retiInstruction;
    arr[static_cast<size_t>(InstructionType::RST)]  = &CentralProcessingUnit::rstInstruction;
    arr[static_cast<size_t>(InstructionType::INC)]  = &CentralProcessingUnit::incInstruction;
    arr[static_cast<size_t>(InstructionType::DEC)]  = &CentralProcessingUnit::decInstruction;
    arr[static_cast<size_t>(InstructionType::ADD)]  = &CentralProcessingUnit::addInstruction;
    arr[static_cast<size_t>(InstructionType::ADC)]  = &CentralProcessingUnit::adcInstruction;
    arr[static_cast<size_t>(InstructionType::SUB)]  = &CentralProcessingUnit::subInstruction;
    arr[static_cast<size_t>(InstructionType::SBC)]  = &CentralProcessingUnit::sbcInstruction;
    arr[static_cast<size_t>(InstructionType::PUSH)] = &CentralProcessingUnit::pushInstruction;
    arr[static_cast<size_t>(InstructionType::POP)]  = &CentralProcessingUnit::popInstruction;
    arr[static_cast<size_t>(InstructionType::EI)]   = &CentralProcessingUnit::eiInstruction;
    arr[static_cast<size_t>(InstructionType::DI)]   = &CentralProcessingUnit::diInstruction;
    arr[static_cast<size_t>(InstructionType::CB)]   = &CentralProcessingUnit::cbInstruction;
    arr[static_cast<size_t>(InstructionType::RLCA)]   = &CentralProcessingUnit::rlcaInstruction;
    arr[static_cast<size_t>(InstructionType::RRCA)]   = &CentralProcessingUnit::rrcaInstruction;
    arr[static_cast<size_t>(InstructionType::RLA)]   = &CentralProcessingUnit::rlaInstruction;
    arr[static_cast<size_t>(InstructionType::RRA)]   = &CentralProcessingUnit::rraInstruction;
    arr[static_cast<size_t>(InstructionType::DAA)]   = &CentralProcessingUnit::daaInstruction;
    arr[static_cast<size_t>(InstructionType::CPL)]   = &CentralProcessingUnit::cplInstruction;
    arr[static_cast<size_t>(InstructionType::SCF)]   = &CentralProcessingUnit::scfInstruction;
    arr[static_cast<size_t>(InstructionType::CCF)]   = &CentralProcessingUnit::ccfInstruction;
    arr[static_cast<size_t>(InstructionType::HALT)]   = &CentralProcessingUnit::haltInstruction;
    arr[static_cast<size_t>(InstructionType::STOP)]   = &CentralProcessingUnit::stopInstruction;
    return arr;
}();

void CentralProcessingUnit::noneInstruction()
{
    Log::print(LogLevel::Error, "None instruction called");
    exit(-1);
}

void CentralProcessingUnit::nopInstruction()
{
}

void CentralProcessingUnit::andInstruction()
{
    u16 value = readRegister(RegisterType::A);
    value &= fetchedData;
    writeRegister(RegisterType::A, value);
    setFlagValues(registers.A == 0, 0, 1, 0);
}

void CentralProcessingUnit::orInstruction()
{
    u16 value = readRegister(RegisterType::A);
    value |= fetchedData;
    writeRegister(RegisterType::A, value);
    setFlagValues(registers.A == 0, 0, 0, 0);
}

void CentralProcessingUnit::xorInstruction()
{
    u16 value = readRegister(RegisterType::A);
    value ^= (fetchedData & 0xFF);
    writeRegister(RegisterType::A, value);
    setFlagValues(registers.A == 0, 0, 0, 0);
}

void CentralProcessingUnit::cpInstruction()
{
    s32 registerA = static_cast<s32>(readRegister(RegisterType::A));
    s32 data = static_cast<s32>(fetchedData);
    s32 value = registerA - data;
    setFlagValues(value == 0, 1, (registerA & 0x0F) - (data & 0x0F) < 0, value < 0);
}

void CentralProcessingUnit::ldInstruction()
{
    if(destinationIsMemory) //Example: LD (BC), A
    {
        if(is16BitsRegister(currentInstruction.register2))
        {
            memoryBus->write16(memoryDestination, fetchedData);
            emulateCycles(1);
        }
        else
            memoryBus->write(memoryDestination, static_cast<u8>(fetchedData));

        emulateCycles(1);
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
}

void CentralProcessingUnit::ldhInstruction()
{
    if(currentInstruction.register1 == RegisterType::A)
    {
        writeRegister(currentInstruction.register1, memoryBus->read(0xFF00 | fetchedData));
    }
    else
    {
        memoryBus->write(memoryDestination, registers.A);
    }
    emulateCycles(1);
}

void CentralProcessingUnit::jpInstruction()
{
    gotoAddress(fetchedData, false);
}

void CentralProcessingUnit::jrInstruction()
{
    s8 relative = static_cast<s8>(fetchedData & 0xFF);
    u16 address = registers.PC + relative;
    gotoAddress(address, false);
}

void CentralProcessingUnit::callInstruction()
{
    gotoAddress(fetchedData, true);
}

void CentralProcessingUnit::retInstruction()
{
    if(currentInstruction.condition != ConditionType::NONE)
        emulateCycles(1);

    if(checkCondition())
    {
        u16 lo = stackPop();
        emulateCycles(1);
        u16 hi = stackPop();
        emulateCycles(1);

        u16 value = (hi << 8) | lo;
        registers.PC = value;
        emulateCycles(1);
    }
}

void CentralProcessingUnit::retiInstruction()
{
    interruptMasterEnabled = true;
    retInstruction();
}

void CentralProcessingUnit::rstInstruction()
{
    gotoAddress(currentInstruction.param, true);
}

void CentralProcessingUnit::popInstruction()
{
    u16 lo = stackPop();
    emulateCycles(1);
    u16 hi = stackPop();
    emulateCycles(1);

    u16 value = (hi << 8) | lo;

    writeRegister(currentInstruction.register1, value);

    if(currentInstruction.register1 == RegisterType::AF)
        writeRegister(currentInstruction.register1, value & 0xFFF0);
}

void CentralProcessingUnit::pushInstruction()
{
    u16 hi = (readRegister(currentInstruction.register1) >> 8) & 0xFF;
    emulateCycles(1);
    stackPush(static_cast<u8>(hi));

    u16 lo = readRegister(currentInstruction.register1) & 0xFF;
    emulateCycles(1);
    stackPush(static_cast<u8>(lo));

    emulateCycles(1);
}

void CentralProcessingUnit::incInstruction()
{
    u16 value = readRegister(currentInstruction.register1) + 1;

    if(is16BitsRegister(currentInstruction.register1))
        emulateCycles(1);

    if(currentInstruction.register1 == RegisterType::HL
        && currentInstruction.addressMode == AddressMode::MR)
    {
        u16 address = readRegister(RegisterType::HL);
        value = fetchedData + 1;
        value &= 0xFF;
        memoryBus->write(address, static_cast<u8>(value));
    }
    else
    {
        writeRegister(currentInstruction.register1, value);
        value = readRegister(currentInstruction.register1);
    }

    if((currentOPCode & 0x03) == 0x03)
        return;

    setFlagValues(value == 0, 0, (value & 0x0F) == 0, -1);
}

void CentralProcessingUnit::decInstruction()
{
    u16 value = readRegister(currentInstruction.register1) - 1;

    if(is16BitsRegister(currentInstruction.register1))
        emulateCycles(1);

    if(currentInstruction.register1 == RegisterType::HL
        && currentInstruction.addressMode == AddressMode::MR)
    {
        u16 address = readRegister(RegisterType::HL);
        value = fetchedData - 1;
        memoryBus->write(address, static_cast<u8>(value));
    }
    else
    {
        writeRegister(currentInstruction.register1, value);
        value = readRegister(currentInstruction.register1);
    }

    if((currentOPCode & 0x0B) == 0x0B)
        return;

    setFlagValues(value == 0, 1, (value & 0x0F) == 0x0F, -1);
}

void CentralProcessingUnit::addInstruction()
{
    u32 value = readRegister(currentInstruction.register1) + fetchedData;
    bool isUsing16BitsRegister = is16BitsRegister(currentInstruction.register1);

    if(isUsing16BitsRegister)
    {
        emulateCycles(1);
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
}

void CentralProcessingUnit::adcInstruction()
{
    u16 data = fetchedData;
    u16 registerA = readRegister(RegisterType::A);
    u16 cFlag = flagC();

    writeRegister(RegisterType::A, (registerA + data + cFlag) & 0xFF);
    setFlagValues(readRegister(RegisterType::A) == 0, 0, (registerA & 0xF) + (data & 0xF) + cFlag > 0xF, registerA + data + cFlag > 0xFF);
}

void CentralProcessingUnit::subInstruction()
{
    u16 value = readRegister(currentInstruction.register1) - fetchedData;
    bool zFlag = value == 0;
    bool hFlag = (static_cast<s32>(readRegister(currentInstruction.register1)) & 0xF) - (static_cast<s32>(fetchedData) & 0xF) < 0;
    bool cFlag = static_cast<s32>(readRegister(currentInstruction.register1)) - static_cast<s32>(fetchedData) < 0;

    writeRegister(currentInstruction.register1, value);
    setFlagValues(zFlag, 1, hFlag, cFlag);
}

void CentralProcessingUnit::sbcInstruction()
{
    u8 value = static_cast<u8>(fetchedData + flagC());
    u16 registerValue = readRegister(currentInstruction.register1) - value;
    bool zFlag = registerValue == 0;
    bool hFlag = (static_cast<s32>(readRegister(currentInstruction.register1)) & 0xF) - (static_cast<s32>(fetchedData) & 0xF) - static_cast<s32>(flagC()) < 0;
    bool cFlag = static_cast<s32>(readRegister(currentInstruction.register1)) - static_cast<s32>(fetchedData) - static_cast<s32>(flagC()) < 0;

    writeRegister(currentInstruction.register1, registerValue);
    setFlagValues(zFlag, 1, hFlag, cFlag);
}

void CentralProcessingUnit::rlcaInstruction()
{
    u8 registerA = static_cast<u8>(readRegister(RegisterType::A));
    bool cFlag = MathUtils<u8>::getBitValue(registerA, 7);
    registerA = (registerA << 1) | static_cast<u8>(cFlag);
    writeRegister(RegisterType::A, registerA);
    setFlagValues(0, 0, 0, cFlag);
}

void CentralProcessingUnit::rrcaInstruction()
{
    u8 registerA = static_cast<u8>(readRegister(RegisterType::A));
    u8 b = static_cast<u8>(MathUtils<u8>::getBitValue(registerA, 0));
    registerA >>= 1;
    registerA |= (b << 7);
    writeRegister(RegisterType::A, registerA);
    setFlagValues(0, 0, 0, b);
}

void CentralProcessingUnit::rlaInstruction()
{
    u8 registerA = static_cast<u8>(readRegister(RegisterType::A));
    bool cFlag = flagC();
    bool cFlagRegisterA = MathUtils<u8>::getBitValue(registerA, 7);

    writeRegister(RegisterType::A, (registerA << 1) | static_cast<u8>(cFlag));
    setFlagValues(0, 0, 0, cFlagRegisterA);
}

void CentralProcessingUnit::rraInstruction()
{
    u8 registerA = static_cast<u8>(readRegister(RegisterType::A));
    bool cFlag = flagC();
    bool newCFlag = MathUtils<u8>::getBitValue(registerA, 0);
    registerA >>= 1;
    registerA |= (cFlag << 7);
    writeRegister(RegisterType::A, registerA);
    setFlagValues(0, 0, 0, newCFlag);
}

void CentralProcessingUnit::daaInstruction()
{
    u16 registerA = readRegister(RegisterType::A);
    u8 value = 0;
    bool cFlag = false;
    
    if(flagH() || (!flagN() && (registerA & 0xF) > 9))
        value = 6;
    
    if(flagC() || (!flagN() && registerA > 0x99))
    {
        value |= 0x60;
        cFlag = true;
    }

    registerA += (flagN() ? -value : value);
    registerA &= 0xFF;
    writeRegister(RegisterType::A, registerA);
    setFlagValues(registerA == 0, -1, 0, cFlag);
}

void CentralProcessingUnit::cplInstruction()
{
    u16 registerA = readRegister(RegisterType::A);
    registerA = ~registerA;
    writeRegister(RegisterType::A, registerA);
    setFlagValues(-1, 1, 1, -1);
}

void CentralProcessingUnit::scfInstruction()
{
    setFlagValues(-1, 0, 0, 1);
}

void CentralProcessingUnit::ccfInstruction()
{
    setFlagValues(-1, 0, 0, flagC() ^ 1);
}

void CentralProcessingUnit::haltInstruction()
{
    halted = true;
}

void CentralProcessingUnit::stopInstruction()
{
    Log::print(LogLevel::Error, "STOP instruction is not implemented yet.");
}

void CentralProcessingUnit::eiInstruction()
{
    enablingInterruptMaster = true;
}

void CentralProcessingUnit::diInstruction()
{
    interruptMasterEnabled = false;
}

void CentralProcessingUnit::gotoAddress(u16 address, bool pushPC)
{
    if(checkCondition())
    {
        if(pushPC)
        {
            stackPush16(registers.PC);
            emulateCycles(2);
        }

        registers.PC = address;
        emulateCycles(1);
    }
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

u8 CentralProcessingUnit::getInterruptFlags() const
{
    return interruptFlags;
}

void CentralProcessingUnit::setInterruptFlags(u8 value)
{
    interruptFlags = value;
}

void CentralProcessingUnit::handleInterrupts()
{
    
}
