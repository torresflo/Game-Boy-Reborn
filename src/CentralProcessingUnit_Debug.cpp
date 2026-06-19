#include "CentralProcessingUnit.h"

#include <array>
#include <format>

RegisterType decodeRegisterForCBInstruction(u8 value);

static const std::array<InstructionType, 8> CBShiftInstructionLookupTable =
{
    InstructionType::RLC,
    InstructionType::RRC,
    InstructionType::RL,
    InstructionType::RR,
    InstructionType::SLA,
    InstructionType::SRA,
    InstructionType::SWAP,
    InstructionType::SRL,
};

std::string CentralProcessingUnit::getFlagsString() const
{
    return std::format("{}{}{}{}",
        flagZ() ? 'Z' : '-', flagN() ? 'N' : '-', flagH() ? 'H' : '-', flagC() ? 'C' : '-');
}

std::string CentralProcessingUnit::getRegistersString() const
{
    return std::format("A: {:02X} F: {} BC: {:02X}{:02X} DE: {:02X}{:02X} HL: {:02X}{:02X}",
        registers.A, getFlagsString(),
        registers.B, registers.C,
        registers.D, registers.E,
        registers.H, registers.L);
}

std::string CentralProcessingUnit::getInstructionString() const
{
    if(currentInstruction.type == InstructionType::CB)
        return getCBInstructionString();

    std::string operands = getInstructionOperandsString();
    if(operands.empty())
        return toString(currentInstruction.type);

    return std::format("{} {}", toString(currentInstruction.type), operands);
}

std::string CentralProcessingUnit::getInstructionOperandsString() const
{
    const RegisterType register1 = currentInstruction.register1;
    const RegisterType register2 = currentInstruction.register2;
    const std::string condition = toString(currentInstruction.condition);

    switch(currentInstruction.addressMode)
    {
        case AddressMode::IMPLY:
            if(currentInstruction.type == InstructionType::RST)
                return std::format("0x{:02X}", currentInstruction.param);
            return condition;

        case AddressMode::R:
            return toString(register1);

        case AddressMode::R_R:
            return std::format("{}, {}", toString(register1), toString(register2));

        case AddressMode::MR_R:
            return std::format("({}), {}", toString(register1), toString(register2));

        case AddressMode::R_MR:
            return std::format("{}, ({})", toString(register1), toString(register2));

        case AddressMode::R_HLI:
            return std::format("{}, (HL+)", toString(register1));

        case AddressMode::R_HLD:
            return std::format("{}, (HL-)", toString(register1));

        case AddressMode::HLI_R:
            return std::format("(HL+), {}", toString(register2));

        case AddressMode::HLD_R:
            return std::format("(HL-), {}", toString(register2));

        case AddressMode::R_D8:
            return std::format("{}, 0x{:02X}", toString(register1), fetchedData & 0xFF);

        case AddressMode::D8:
        {
            if(register1 != RegisterType::NONE) //Example: XOR A, 0x05
                return std::format("{}, 0x{:02X}", toString(register1), fetchedData & 0xFF);

            //Relative jump (JR): target was already resolved like jrInstruction() would
            u16 target = registers.PC + static_cast<s8>(fetchedData & 0xFF);
            std::string targetString = std::format("0x{:04X}", target);
            return condition.empty() ? targetString : std::format("{}, {}", condition, targetString);
        }

        case AddressMode::R_D16:
            return std::format("{}, 0x{:04X}", toString(register1), fetchedData);

        case AddressMode::D16:
        {
            std::string targetString = std::format("0x{:04X}", fetchedData);
            return condition.empty() ? targetString : std::format("{}, {}", condition, targetString);
        }

        case AddressMode::MR_D8:
            return std::format("({}), 0x{:02X}", toString(register1), fetchedData & 0xFF);

        case AddressMode::MR:
            return std::format("({})", toString(register1));

        case AddressMode::A16_R:
        case AddressMode::D16_R:
            return std::format("(0x{:04X}), {}", memoryDestination, toString(register2));

        case AddressMode::R_A16:
        {
            //The address bytes were already consumed from PC; read them back for display
            u16 address = static_cast<u16>(memoryBus->read(registers.PC - 2))
                | (static_cast<u16>(memoryBus->read(registers.PC - 1)) << 8);
            return std::format("{}, (0x{:04X})", toString(register1), address);
        }

        case AddressMode::R_A8:
            return std::format("{}, (0x{:04X})", toString(register1), 0xFF00 | (fetchedData & 0xFF));

        case AddressMode::A8_R:
            return std::format("(0x{:04X}), {}", memoryDestination, toString(register2));

        case AddressMode::HL_SPR:
        {
            s8 offset = static_cast<s8>(fetchedData & 0xFF);
            int absOffset = offset < 0 ? -static_cast<int>(offset) : static_cast<int>(offset);
            return std::format("{}, {}{}{}", toString(register1), toString(register2), offset < 0 ? "-" : "+", absOffset);
        }

        default:
            Log::print(LogLevel::Error, "Unimplemented address mode for instruction string.");
            return "";
    }
}

std::string CentralProcessingUnit::getCBInstructionString() const
{
    u8 operation = static_cast<u8>(fetchedData);
    RegisterType registerType = decodeRegisterForCBInstruction(operation & 0b111);
    u8 bitPosition = (operation >> 3) & 0b111;
    u8 bitOperation = (operation >> 6) & 0b11;

    std::string registerString = registerType == RegisterType::HL ? "(HL)" : toString(registerType);

    switch(bitOperation)
    {
        case 1: //BIT
            return std::format("BIT {}, {}", bitPosition, registerString);
        case 2: //RES
            return std::format("RES {}, {}", bitPosition, registerString);
        case 3: //SET
            return std::format("SET {}, {}", bitPosition, registerString);
        default: //Rotate/shift group
            return std::format("{} {}", toString(CBShiftInstructionLookupTable[bitPosition]), registerString);
    }
}
