#include "DisassemblyPanel.h"

#include <array>
#include <format>

#include <imgui.h>

#include "GameBoyEmulator.h"
#include "MemoryBus.h"
#include "CentralProcessingUnit.h"
#include "InstructionDefinitions.h"

DisassemblyPanel::DisassemblyPanel()
    : DebugPanel("Disassembly")
{
}

DisassemblyPanel::DecodedInstruction DisassemblyPanel::decodeInstruction(const MemoryBus& bus, u16 address) const
{
    const u8 opcode = bus.read(address);
    const InstructionData& data = Instructions[opcode];
    const u8 lo = bus.read(address + 1);
    const u8 hi = bus.read(address + 2);

    u8 length = 1;
    switch(data.addressMode)
    {
        case AddressMode::R_D8:
        case AddressMode::D8:
        case AddressMode::R_A8:
        case AddressMode::A8_R:
        case AddressMode::MR_D8:
        case AddressMode::HL_SPR:
            length = 2;
            break;

        case AddressMode::R_D16:
        case AddressMode::D16:
        case AddressMode::D16_R:
        case AddressMode::A16_R:
        case AddressMode::R_A16:
            length = 3;
            break;

        default:
            if(data.type == InstructionType::CB)
                length = 2;
            break;
    }

    std::string bytes;
    switch(length)
    {
        case 2: bytes = std::format("{:02X} {:02X}", opcode, lo); break;
        case 3: bytes = std::format("{:02X} {:02X} {:02X}", opcode, lo, hi); break;
        default: bytes = std::format("{:02X}", opcode); break;
    }

    std::string mnemonic;
    if(data.type == InstructionType::CB)
    {
        mnemonic = formatCBInstruction(lo);
    }
    else
    {
        std::string operands = formatOperands(data, bus, address);
        mnemonic = operands.empty()
            ? toString(data.type)
            : std::format("{} {}", toString(data.type), operands);
    }

    return {bytes, mnemonic, length};
}

std::string DisassemblyPanel::formatOperands(const InstructionData& data, const MemoryBus& bus, u16 address) const
{
    const u8 lo = bus.read(address + 1);
    const u8 hi = bus.read(address + 2);
    const u16 immediate16 = static_cast<u16>((hi << 8) | lo);

    const std::string reg1 = toString(data.register1);
    const std::string reg2 = toString(data.register2);
    const std::string condition = toString(data.condition);

    switch(data.addressMode)
    {
        case AddressMode::IMPLY:
            if(data.type == InstructionType::RST)
                return std::format("0x{:02X}", data.param);
            return condition;

        case AddressMode::R:
            return reg1;

        case AddressMode::R_R:
            return std::format("{}, {}", reg1, reg2);

        case AddressMode::MR_R:
            return std::format("({}), {}", reg1, reg2);

        case AddressMode::R_MR:
            return std::format("{}, ({})", reg1, reg2);

        case AddressMode::R_HLI:
            return std::format("{}, (HL+)", reg1);

        case AddressMode::R_HLD:
            return std::format("{}, (HL-)", reg1);

        case AddressMode::HLI_R:
            return std::format("(HL+), {}", reg2);

        case AddressMode::HLD_R:
            return std::format("(HL-), {}", reg2);

        case AddressMode::MR:
            return std::format("({})", reg1);

        case AddressMode::R_D8:
            return std::format("{}, 0x{:02X}", reg1, lo);

        case AddressMode::D8:
        {
            if(data.register1 != RegisterType::NONE)
                return std::format("{}, 0x{:02X}", reg1, lo);
            u16 target = address + 2 + static_cast<u16>(static_cast<s8>(lo));
            std::string targetString = std::format("0x{:04X}", target);
            return condition.empty() ? targetString : std::format("{}, {}", condition, targetString);
        }

        case AddressMode::MR_D8:
            return std::format("({}), 0x{:02X}", reg1, lo);

        case AddressMode::R_A8:
            return std::format("{}, (0x{:04X})", reg1, 0xFF00 | lo);

        case AddressMode::A8_R:
            return std::format("(0x{:04X}), {}", 0xFF00 | lo, reg2);

        case AddressMode::HL_SPR:
        {
            const s8 offset = static_cast<s8>(lo);
            const int absOffset = offset < 0 ? -static_cast<int>(offset) : static_cast<int>(offset);
            return std::format("{}, {}{}{}", reg1, reg2, offset < 0 ? "-" : "+", absOffset);
        }

        case AddressMode::R_D16:
            return std::format("{}, 0x{:04X}", reg1, immediate16);

        case AddressMode::D16:
        {
            std::string targetString = std::format("0x{:04X}", immediate16);
            return condition.empty() ? targetString : std::format("{}, {}", condition, targetString);
        }

        case AddressMode::D16_R:
        case AddressMode::A16_R:
            return std::format("(0x{:04X}), {}", immediate16, reg2);

        case AddressMode::R_A16:
            return std::format("{}, (0x{:04X})", reg1, immediate16);

        default:
            return "";
    }
}

std::string DisassemblyPanel::formatCBInstruction(u8 cbByte) const
{
    static constexpr std::array<const char*, 8> RegisterNames =
        {"B", "C", "D", "E", "H", "L", "(HL)", "A"};

    static constexpr std::array<InstructionType, 8> ShiftInstructions =
    {
        InstructionType::RLC, InstructionType::RRC,
        InstructionType::RL,  InstructionType::RR,
        InstructionType::SLA, InstructionType::SRA,
        InstructionType::SWAP, InstructionType::SRL,
    };

    const u8 operation   = (cbByte >> 6) & 0x03;
    const u8 bitPosition = (cbByte >> 3) & 0x07;
    const u8 registerIndex = cbByte & 0x07;
    const char* registerName = RegisterNames[registerIndex];

    switch(operation)
    {
        case 1: return std::format("BIT {}, {}", bitPosition, registerName);
        case 2: return std::format("RES {}, {}", bitPosition, registerName);
        case 3: return std::format("SET {}, {}", bitPosition, registerName);
        default: return std::format("{} {}", toString(ShiftInstructions[bitPosition]), registerName);
    }
}

void DisassemblyPanel::draw(GameBoyEmulator& emulator)
{
    viewAddress = emulator.getCPU().getRegisters().PC;

    constexpr ImGuiTableFlags tableFlags =
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_SizingFixedFit |
        ImGuiTableFlags_BordersInnerV;

    if(ImGui::BeginTable("##disasm", 3, tableFlags))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 70.f);
        ImGui::TableSetupColumn("Bytes", ImGuiTableColumnFlags_WidthFixed, 80.f);
        ImGui::TableSetupColumn("Instruction", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        const MemoryBus& bus = emulator.getMemoryBus();
        u16 address = viewAddress;

        for(int index = 0; index < InstructionCount; ++index)
        {
            const DecodedInstruction instruction = decodeInstruction(bus, address);

            ImGui::TableNextRow();

            if(index == 0)
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::ColorConvertFloat4ToU32(PCHighlightColor));

            ImGui::TableNextColumn();
            ImGui::TextColored(AddressColor, "0x%04X", address);

            ImGui::TableNextColumn();
            ImGui::TextColored(BytesColor, "%s", instruction.bytes.c_str());

            ImGui::TableNextColumn();
            ImGui::TextColored(MnemonicColor, "%s", instruction.mnemonic.c_str());

            address += instruction.length;
        }

        ImGui::EndTable();
    }
}
