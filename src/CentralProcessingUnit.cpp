#include "CentralProcessingUnit.h"

#include <format>

#include "MathUtils.h"

const std::array<CentralProcessingUnit::InstructionFunc, static_cast<size_t>(InstructionType::COUNT)> CentralProcessingUnit::InstructionFuncs = []() {
    std::array<CentralProcessingUnit::InstructionFunc, static_cast<size_t>(InstructionType::COUNT)> arr{};
    arr[static_cast<size_t>(InstructionType::NONE)] = &CentralProcessingUnit::noneInstruction;
    arr[static_cast<size_t>(InstructionType::NOP)]  = &CentralProcessingUnit::nopInstruction;
    arr[static_cast<size_t>(InstructionType::LD)]   = &CentralProcessingUnit::ldInstruction;
    arr[static_cast<size_t>(InstructionType::JP)]   = &CentralProcessingUnit::jpInstruction;
    arr[static_cast<size_t>(InstructionType::DI)]   = &CentralProcessingUnit::diInstruction;
    arr[static_cast<size_t>(InstructionType::XOR)]   = &CentralProcessingUnit::xorInstruction;
    return arr;
}();

void CentralProcessingUnit::initialize(MemoryBus* bus)
{
    memoryBus = bus;

    registers = {0};
    registers.PC = 0x100;
    registers.A = 0x01;
}

u8 CentralProcessingUnit::step()
{
    u8 consumedCycles = 0;
    if(!halted)
    {
        u16 pc = registers.PC;

        fetchInstruction();
        consumedCycles += fetchData();
        
        Log::print(LogLevel::Debug,  std::format("{:04X} -> {:<4s} ({:02X} {:02X} {:02X}) A: {:02X} B: {:02X} C: {:02X}",
            pc, toString(currentInstruction.type), currentOPCode,
            memoryBus->read(pc + 1), memoryBus->read(pc + 2),
            registers.A, registers.B, registers.C));

        consumedCycles += execute();
    }

    return consumedCycles;
}

void CentralProcessingUnit::fetchInstruction()
{
    currentOPCode = memoryBus->read(registers.PC);
    registers.PC++;
    currentInstruction = getInstructionFromOpCode(currentOPCode);
}

u8 CentralProcessingUnit::fetchData()
{
    memoryDestination = 0;
    destinationIsMemory = false;
    u8 consumedCycles = 0;

    switch(currentInstruction.addressMode)
    {
        case AdressMode::IMPLY:
            break;
        case AdressMode::R:
            fetchedData = readRegister(currentInstruction.register1);
            break;
        case AdressMode::R_D8:
            fetchedData = memoryBus->read(registers.PC);
            consumedCycles++;
            registers.PC++;
            break;
        case AdressMode::D16:
        {
            u16 low = memoryBus->read(registers.PC);
            consumedCycles++;
            u16 high = memoryBus->read(registers.PC + 1);
            consumedCycles++;
            fetchedData = (high << 8) | low;
            registers.PC += 2;
            break;
        }
        default:
            Log::print(LogLevel::Error, "Unimplemented address mode.");
            break;
    }

    return consumedCycles;
}

u8 CentralProcessingUnit::execute()
{
    InstructionFunc instructionFunc = getInstructionFunc(currentInstruction.type);
    if(instructionFunc == nullptr)
    {
        Log::print(LogLevel::Error, "Unimplemented instruction: ", toString(currentInstruction.type), " (", std::format("{:02X}", currentOPCode), ")");
        exit(-1);
    }
    return (this->*instructionFunc)();
}

const Instruction &CentralProcessingUnit::getInstructionFromOpCode(u8 opcode)
{
    return Instructions[opcode];
}

u16 CentralProcessingUnit::readRegister(RegisterType type)
{
    switch (type)
    {
        case RegisterType::A:
            return registers.A;
        case RegisterType::F:
            return registers.F;
        case RegisterType::B:
            return registers.B;
        case RegisterType::C:
            return registers.C;
        case RegisterType::D:
            return registers.D;
        case RegisterType::E:
            return registers.E;
        case RegisterType::H:
            return registers.H;
        case RegisterType::L:
            return registers.L;
        case RegisterType::AF:
            return reverse(*(u16*)(&registers.A));
        case RegisterType::BC:
            return reverse(*(u16*)(&registers.B));
        case RegisterType::DE:
            return reverse(*(u16*)(&registers.D));
        case RegisterType::HL:
            return reverse(*(u16*)(&registers.H));
        case RegisterType::SP:
            return registers.SP;
        case RegisterType::PC:
            return registers.PC;
        default:
            Log::print(LogLevel::Error, "Unimplemented register type");
            return 0;
    }
}

u16 CentralProcessingUnit::reverse(u16 value) const
{
    return (value & 0xFF00) >> 8 | (value & 0x00FF) << 8;
}

CentralProcessingUnit::InstructionFunc CentralProcessingUnit::getInstructionFunc(InstructionType type)
{
    return InstructionFuncs[static_cast<size_t>(type)];
}

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
    return 0;
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
