#include "CentralProcessingUnit.h"

#include <format>

void CentralProcessingUnit::initialize(MemoryBus* bus)
{
    memoryBus = bus;

    registers.PC = 0x100;
}

u8 CentralProcessingUnit::step()
{
    u8 consumedCycles = 0;
    if(!halted)
    {
        u16 pc = registers.PC;

        fetchInstruction();
        consumedCycles = fetchData();
        execute();
    
        Log::print(LogLevel::Debug, "Executing opcode: ", std::format("{:02X}", currentOPCode), " - PC = ", std::format("{:04X}", pc));
    }

    return consumedCycles;
}

void CentralProcessingUnit::fetchInstruction()
{
    currentOPCode = memoryBus->read(registers.PC);
    registers.PC++;
    currentInstruction = getInstructionFromOpCode(currentOPCode);

    if(currentInstruction.type == InstructionType::NONE)
    {
        Log::print(LogLevel::Error, "Invalid opcode: ", std::format("{:02X}", currentOPCode));
        exit(-1);
    }
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
            //fetchData = read register 
            break;
        case AdressMode::R_D8:
            data = memoryBus->read(registers.PC);
            consumedCycles++;
            registers.PC++;
            break;
        case AdressMode::D16:
        {
            u16 low = memoryBus->read(registers.PC);
            consumedCycles++;
            u16 high = memoryBus->read(registers.PC + 1);
            consumedCycles++;
            data = (high << 8) | low;
            registers.PC += 2;
            break;
        }
        default:
            NO_IMPLEMENTATION
            break;
    }

    return consumedCycles;
}

void CentralProcessingUnit::execute()
{
}

const Instruction &CentralProcessingUnit::getInstructionFromOpCode(u8 opcode)
{
    auto it = Instructions.find(opcode);
    if(it != Instructions.end())
        return it->second;

    static Instruction none;
    return none;
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
            NO_IMPLEMENTATION
            return 0;
    }
}

u16 CentralProcessingUnit::reverse(u16 value) const
{
    return (value & 0xFF00) >> 8 | (value & 0x00FF) << 8;
}
