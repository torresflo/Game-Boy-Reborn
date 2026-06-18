#include "CentralProcessingUnit.h"

#include <format>

void CentralProcessingUnit::initialize(MemoryBus* bus)
{
    memoryBus = bus;

    registers = {0};
    registers.PC = 0x100;
    registers.A = 0x01;
}

void CentralProcessingUnit::step()
{
    if(!halted)
    {
        u16 pc = registers.PC;

        fetchInstruction();
        emulateCycles(1);
        fetchData();

        Log::print(LogLevel::Debug, std::format("[{:08d}] {:04X} -> {:<6s} ({:02X} {:02X} {:02X}) A: {:02X} F: {:c}{:c}{:c}{:c} BC: {:02X}{:02X} DE: {:02X}{:02X} HL: {:02X}{:02X}",
            cycles, pc, toString(currentInstruction.type), currentOPCode,
            memoryBus->read(pc + 1), memoryBus->read(pc + 2),
            registers.A,
            flagZ() ? 'Z' : '-', flagN() ? 'N' : '-', flagH() ? 'H' : '-', flagC() ? 'C' : '-',
            registers.B, registers.C, registers.D, registers.E, registers.H, registers.L));

        execute();
    }
    else
    {
        emulateCycles(1);

        if(interruptFlags != 0)
            halted = false;
    }

    if(interruptMasterEnabled)
    {
        handleInterrupts();
        enablingInterruptMaster = false;
    }

    if(enablingInterruptMaster)
    {
        interruptMasterEnabled = true;
    }
}

void CentralProcessingUnit::fetchInstruction()
{
    currentOPCode = memoryBus->read(registers.PC);
    registers.PC++;
    currentInstruction = getInstructionFromOpCode(currentOPCode);
}

void CentralProcessingUnit::execute()
{
    InstructionFunc instructionFunc = getInstructionFunc(currentInstruction.type);
    if(instructionFunc == nullptr)
    {
        Log::print(LogLevel::Error, "Unimplemented instruction: ", toString(currentInstruction.type), " (", std::format("{:02X}", currentOPCode), ")");
        exit(-1);
    }
    (this->*instructionFunc)();
}

const InstructionData& CentralProcessingUnit::getInstructionFromOpCode(u8 opcode) const
{
    return Instructions[opcode];
}

u16 CentralProcessingUnit::readRegister(RegisterType type) const
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
        case RegisterType::NONE:
            Log::print(LogLevel::Error, "Trying to read in register of type <NONE>!");
            return 0;
        default:
            Log::print(LogLevel::Error, "Unimplemented register type (read)");
            return 0;
    }
}

void CentralProcessingUnit::writeRegister(RegisterType type, u16 value)
{
    switch (type)
    {
        case RegisterType::A:
            registers.A = static_cast<u8>(value);
            break;
        case RegisterType::F:
            registers.F = static_cast<u8>(value);
            break;
        case RegisterType::B:
            registers.B = static_cast<u8>(value);
            break;
        case RegisterType::C:
            registers.C = static_cast<u8>(value);
            break;
        case RegisterType::D:
            registers.D = static_cast<u8>(value);
            break;
        case RegisterType::E:
            registers.E = static_cast<u8>(value);
            break;
        case RegisterType::H:
            registers.H = static_cast<u8>(value);
            break;
        case RegisterType::L:
            registers.L = static_cast<u8>(value);
            break;
        case RegisterType::AF:
            *(u16*)(&registers.A) = reverse(value);
            break;
        case RegisterType::BC:
            *(u16*)(&registers.B) = reverse(value);
            break;
        case RegisterType::DE:
            *(u16*)(&registers.D) = reverse(value);
            break;
        case RegisterType::HL:
            *(u16*)(&registers.H) = reverse(value);
            break;
        case RegisterType::SP:
            registers.SP = value;
            break;
        case RegisterType::PC:
            registers.PC = value;
            break;
        case RegisterType::NONE:
            Log::print(LogLevel::Error, "Trying to write in register of type <NONE>!");
            break;
        default:
            Log::print(LogLevel::Error, "Unimplemented register type (write)");
            break;
    }
}

u16 CentralProcessingUnit::reverse(u16 value) const
{
    return (value & 0xFF00) >> 8 | (value & 0x00FF) << 8;
}

void CentralProcessingUnit::emulateCycles([[maybe_unused]] u8 cycleCount)
{
}

CentralProcessingUnit::InstructionFunc CentralProcessingUnit::getInstructionFunc(InstructionType type)
{
    return InstructionFuncs[static_cast<size_t>(type)];
}

bool CentralProcessingUnit::is16BitsRegister(RegisterType type) const
{
    return type >= RegisterType::AF;
}
