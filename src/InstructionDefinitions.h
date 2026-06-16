#pragma once

#include <array>

#include "Common.h"

enum class AddressMode
{
    IMPLY,
    R_D16,
    R_R,
    MR_R,
    R,
    R_D8,
    R_MR,
    R_HLI,
    R_HLD,
    HLI_R,
    HLD_R,
    R_A8,
    A8_R,
    HL_SPR,
    D16,
    D8,
    D16_R,
    MR_D8,
    MR,
    A16_R,
    R_A16
};

enum class RegisterType
{
    NONE,
    A,
    F,
    B,
    C,
    D,
    E,
    H,
    L,
    AF,
    BC,
    DE,
    HL,
    SP,
    PC
};

enum class InstructionType
{
    NONE,
    NOP,
    LD,
    INC,
    DEC,
    RLCA,
    ADD,
    RRCA,
    STOP,
    RLA,
    JR,
    RRA,
    DAA,
    CPL,
    SCF,
    CCF,
    HALT,
    ADC,
    SUB,
    SBC,
    AND,
    XOR,
    OR,
    CP,
    POP,
    JP,
    PUSH,
    RET,
    CB,
    CALL,
    RETI,
    LDH,
    JPHL,
    DI,
    EI,
    RST,
    ERR,
    //CB instructions
    RLC,
    RRC,
    RL,
    RR,
    SLA,
    SRA,
    SWAP,
    SRL,
    BIT,
    RES,
    SET,
    COUNT
};

inline const char* toString(InstructionType type)
{
    switch(type)
    {
        case InstructionType::NONE:  return "<NONE>";
        case InstructionType::NOP:   return "NOP";
        case InstructionType::LD:    return "LD";
        case InstructionType::INC:   return "INC";
        case InstructionType::DEC:   return "DEC";
        case InstructionType::RLCA:  return "RLCA";
        case InstructionType::ADD:   return "ADD";
        case InstructionType::RRCA:  return "RRCA";
        case InstructionType::STOP:  return "STOP";
        case InstructionType::RLA:   return "RLA";
        case InstructionType::JR:    return "JR";
        case InstructionType::RRA:   return "RRA";
        case InstructionType::DAA:   return "DAA";
        case InstructionType::CPL:   return "CPL";
        case InstructionType::SCF:   return "SCF";
        case InstructionType::CCF:   return "CCF";
        case InstructionType::HALT:  return "HALT";
        case InstructionType::ADC:   return "ADC";
        case InstructionType::SUB:   return "SUB";
        case InstructionType::SBC:   return "SBC";
        case InstructionType::AND:   return "AND";
        case InstructionType::XOR:   return "XOR";
        case InstructionType::OR:    return "OR";
        case InstructionType::CP:    return "CP";
        case InstructionType::POP:   return "POP";
        case InstructionType::JP:    return "JP";
        case InstructionType::PUSH:  return "PUSH";
        case InstructionType::RET:   return "RET";
        case InstructionType::CB:    return "CB";
        case InstructionType::CALL:  return "CALL";
        case InstructionType::RETI:  return "RETI";
        case InstructionType::LDH:   return "LDH";
        case InstructionType::JPHL:  return "JPHL";
        case InstructionType::DI:    return "DI";
        case InstructionType::EI:    return "EI";
        case InstructionType::RST:   return "RST";
        case InstructionType::ERR:   return "ERR";
        case InstructionType::RLC:   return "RLC";
        case InstructionType::RRC:   return "RRC";
        case InstructionType::RL:    return "RL";
        case InstructionType::RR:    return "RR";
        case InstructionType::SLA:   return "SLA";
        case InstructionType::SRA:   return "SRA";
        case InstructionType::SWAP:  return "SWAP";
        case InstructionType::SRL:   return "SRL";
        case InstructionType::BIT:   return "BIT";
        case InstructionType::RES:   return "RES";
        case InstructionType::SET:   return "SET";
        case InstructionType::COUNT: return "COUNT";
    }
    return "UNKNOWN";
}

enum class ConditionType
{
    NONE,
    NZ,
    Z,
    NC,
    C
};

struct InstructionData
{
    InstructionType type = InstructionType::NONE;
    AddressMode addressMode = AddressMode::IMPLY;
    RegisterType register1 = RegisterType::NONE;
    RegisterType register2 = RegisterType::NONE;
    ConditionType condition = ConditionType::NONE;
    u8 param = 0;
};

inline const std::array<InstructionData, 256> Instructions = []() {
    std::array<InstructionData, 256> arr{};
    //0x0x
    arr[0x00] = {InstructionType::NOP};
    arr[0x01] = {InstructionType::LD, AddressMode::R_D16, RegisterType::BC};
    arr[0x02] = {InstructionType::LD, AddressMode::MR_R, RegisterType::BC, RegisterType::A};
    arr[0x05] = {InstructionType::DEC, AddressMode::R, RegisterType::B};
    arr[0x06] = {InstructionType::LD, AddressMode::R_D8, RegisterType::B};
    arr[0x08] = {InstructionType::LD, AddressMode::A16_R, RegisterType::NONE, RegisterType::SP};
    arr[0x0A] = {InstructionType::LD, AddressMode::R_MR, RegisterType::A, RegisterType::BC};
    arr[0x0E] = {InstructionType::LD, AddressMode::R_D8, RegisterType::C};

    //0x1x
    arr[0x11] = {InstructionType::LD, AddressMode::R_D16, RegisterType::DE};
    arr[0x12] = {InstructionType::LD, AddressMode::MR_R, RegisterType::DE, RegisterType::A};
    arr[0x15] = {InstructionType::DEC, AddressMode::R, RegisterType::D};
    arr[0x16] = {InstructionType::LD, AddressMode::R_D8, RegisterType::D};
    arr[0x18] = {InstructionType::JR, AddressMode::D8};
    arr[0x1A] = {InstructionType::LD, AddressMode::R_MR, RegisterType::A, RegisterType::DE};
    arr[0x1E] = {InstructionType::LD, AddressMode::R_D8, RegisterType::E};

    //0x2x
    arr[0x20] = {InstructionType::JR, AddressMode::D8, RegisterType::NONE, RegisterType::NONE, ConditionType::NZ};
    arr[0x21] = {InstructionType::LD, AddressMode::R_D16, RegisterType::HL};
    arr[0x22] = {InstructionType::LD, AddressMode::HLI_R, RegisterType::HL, RegisterType::A};
    arr[0x25] = {InstructionType::DEC, AddressMode::R, RegisterType::H};
    arr[0x26] = {InstructionType::LD, AddressMode::R_D8, RegisterType::H};
    arr[0x28] = {InstructionType::JR, AddressMode::D8, RegisterType::NONE, RegisterType::NONE, ConditionType::Z};
    arr[0x2A] = {InstructionType::LD, AddressMode::R_HLI, RegisterType::A, RegisterType::HL};
    arr[0x2E] = {InstructionType::LD, AddressMode::R_D8, RegisterType::L};

    //0x3x
    arr[0x30] = {InstructionType::JR, AddressMode::D8, RegisterType::NONE, RegisterType::NONE, ConditionType::NC};
    arr[0x31] = {InstructionType::LD, AddressMode::R_D16, RegisterType::SP};
    arr[0x32] = {InstructionType::LD, AddressMode::HLD_R, RegisterType::HL, RegisterType::A};
    arr[0x35] = {InstructionType::DEC, AddressMode::MR, RegisterType::HL};
    arr[0x36] = {InstructionType::LD, AddressMode::MR_D8, RegisterType::HL};
    arr[0x38] = {InstructionType::JR, AddressMode::D8, RegisterType::NONE, RegisterType::NONE, ConditionType::C};
    arr[0x3A] = {InstructionType::LD, AddressMode::R_HLD, RegisterType::A, RegisterType::HL};
    arr[0x3E] = {InstructionType::LD, AddressMode::R_D8, RegisterType::A};

    //0x4x
    arr[0x40] = {InstructionType::LD, AddressMode::R_R, RegisterType::B, RegisterType::B};
    arr[0x41] = {InstructionType::LD, AddressMode::R_R, RegisterType::B, RegisterType::C};
    arr[0x42] = {InstructionType::LD, AddressMode::R_R, RegisterType::B, RegisterType::D};
    arr[0x43] = {InstructionType::LD, AddressMode::R_R, RegisterType::B, RegisterType::E};
    arr[0x44] = {InstructionType::LD, AddressMode::R_R, RegisterType::B, RegisterType::H};
    arr[0x45] = {InstructionType::LD, AddressMode::R_R, RegisterType::B, RegisterType::L};
    arr[0x46] = {InstructionType::LD, AddressMode::R_MR, RegisterType::B, RegisterType::HL};
    arr[0x47] = {InstructionType::LD, AddressMode::R_R, RegisterType::B, RegisterType::A};
    arr[0x48] = {InstructionType::LD, AddressMode::R_R, RegisterType::C, RegisterType::B};
    arr[0x49] = {InstructionType::LD, AddressMode::R_R, RegisterType::C, RegisterType::C};
    arr[0x4A] = {InstructionType::LD, AddressMode::R_R, RegisterType::C, RegisterType::D};
    arr[0x4B] = {InstructionType::LD, AddressMode::R_R, RegisterType::C, RegisterType::E};
    arr[0x4C] = {InstructionType::LD, AddressMode::R_R, RegisterType::C, RegisterType::H};
    arr[0x4D] = {InstructionType::LD, AddressMode::R_R, RegisterType::C, RegisterType::L};
    arr[0x4E] = {InstructionType::LD, AddressMode::R_MR, RegisterType::C, RegisterType::HL};
    arr[0x4F] = {InstructionType::LD, AddressMode::R_R, RegisterType::C, RegisterType::A};

    //0x5x
    arr[0x50] = {InstructionType::LD, AddressMode::R_R, RegisterType::D, RegisterType::B};
    arr[0x51] = {InstructionType::LD, AddressMode::R_R, RegisterType::D, RegisterType::C};
    arr[0x52] = {InstructionType::LD, AddressMode::R_R, RegisterType::D, RegisterType::D};
    arr[0x53] = {InstructionType::LD, AddressMode::R_R, RegisterType::D, RegisterType::E};
    arr[0x54] = {InstructionType::LD, AddressMode::R_R, RegisterType::D, RegisterType::H};
    arr[0x55] = {InstructionType::LD, AddressMode::R_R, RegisterType::D, RegisterType::L};
    arr[0x56] = {InstructionType::LD, AddressMode::R_MR, RegisterType::D, RegisterType::HL};
    arr[0x57] = {InstructionType::LD, AddressMode::R_R, RegisterType::D, RegisterType::A};
    arr[0x58] = {InstructionType::LD, AddressMode::R_R, RegisterType::E, RegisterType::B};
    arr[0x59] = {InstructionType::LD, AddressMode::R_R, RegisterType::E, RegisterType::C};
    arr[0x5A] = {InstructionType::LD, AddressMode::R_R, RegisterType::E, RegisterType::D};
    arr[0x5B] = {InstructionType::LD, AddressMode::R_R, RegisterType::E, RegisterType::E};
    arr[0x5C] = {InstructionType::LD, AddressMode::R_R, RegisterType::E, RegisterType::H};
    arr[0x5D] = {InstructionType::LD, AddressMode::R_R, RegisterType::E, RegisterType::L};
    arr[0x5E] = {InstructionType::LD, AddressMode::R_MR, RegisterType::E, RegisterType::HL};
    arr[0x5F] = {InstructionType::LD, AddressMode::R_R, RegisterType::E, RegisterType::A};

    //0x6x
    arr[0x60] = {InstructionType::LD, AddressMode::R_R, RegisterType::H, RegisterType::B};
    arr[0x61] = {InstructionType::LD, AddressMode::R_R, RegisterType::H, RegisterType::C};
    arr[0x62] = {InstructionType::LD, AddressMode::R_R, RegisterType::H, RegisterType::D};
    arr[0x63] = {InstructionType::LD, AddressMode::R_R, RegisterType::H, RegisterType::E};
    arr[0x64] = {InstructionType::LD, AddressMode::R_R, RegisterType::H, RegisterType::H};
    arr[0x65] = {InstructionType::LD, AddressMode::R_R, RegisterType::H, RegisterType::L};
    arr[0x66] = {InstructionType::LD, AddressMode::R_MR, RegisterType::H, RegisterType::HL};
    arr[0x67] = {InstructionType::LD, AddressMode::R_R, RegisterType::H, RegisterType::A};
    arr[0x68] = {InstructionType::LD, AddressMode::R_R, RegisterType::L, RegisterType::B};
    arr[0x69] = {InstructionType::LD, AddressMode::R_R, RegisterType::L, RegisterType::C};
    arr[0x6A] = {InstructionType::LD, AddressMode::R_R, RegisterType::L, RegisterType::D};
    arr[0x6B] = {InstructionType::LD, AddressMode::R_R, RegisterType::L, RegisterType::E};
    arr[0x6C] = {InstructionType::LD, AddressMode::R_R, RegisterType::L, RegisterType::H};
    arr[0x6D] = {InstructionType::LD, AddressMode::R_R, RegisterType::L, RegisterType::L};
    arr[0x6E] = {InstructionType::LD, AddressMode::R_MR, RegisterType::L, RegisterType::HL};
    arr[0x6F] = {InstructionType::LD, AddressMode::R_R, RegisterType::L, RegisterType::A};

    //0x7x
    arr[0x70] = {InstructionType::LD, AddressMode::MR_R, RegisterType::HL, RegisterType::B};
    arr[0x71] = {InstructionType::LD, AddressMode::MR_R, RegisterType::HL, RegisterType::C};
    arr[0x72] = {InstructionType::LD, AddressMode::MR_R, RegisterType::HL, RegisterType::D};
    arr[0x73] = {InstructionType::LD, AddressMode::MR_R, RegisterType::HL, RegisterType::E};
    arr[0x74] = {InstructionType::LD, AddressMode::MR_R, RegisterType::HL, RegisterType::H};
    arr[0x75] = {InstructionType::LD, AddressMode::MR_R, RegisterType::HL, RegisterType::L};
    arr[0x76] = {InstructionType::HALT};
    arr[0x77] = {InstructionType::LD, AddressMode::MR_R, RegisterType::HL, RegisterType::A};
    arr[0x78] = {InstructionType::LD, AddressMode::R_R, RegisterType::A, RegisterType::B};
    arr[0x79] = {InstructionType::LD, AddressMode::R_R, RegisterType::A, RegisterType::C};
    arr[0x7A] = {InstructionType::LD, AddressMode::R_R, RegisterType::A, RegisterType::D};
    arr[0x7B] = {InstructionType::LD, AddressMode::R_R, RegisterType::A, RegisterType::E};
    arr[0x7C] = {InstructionType::LD, AddressMode::R_R, RegisterType::A, RegisterType::H};
    arr[0x7D] = {InstructionType::LD, AddressMode::R_R, RegisterType::A, RegisterType::L};
    arr[0x7E] = {InstructionType::LD, AddressMode::R_MR, RegisterType::A, RegisterType::HL};
    arr[0x7F] = {InstructionType::LD, AddressMode::R_R, RegisterType::A, RegisterType::A};

    //0xAx
    arr[0xAF] = {InstructionType::XOR, AddressMode::R, RegisterType::A};
    
    //0xCx
    arr[0xC0] = {InstructionType::RET, AddressMode::IMPLY, RegisterType::NONE, RegisterType::NONE, ConditionType::NZ};
    arr[0xC1] = {InstructionType::POP, AddressMode::R, RegisterType::BC};
    arr[0xC2] = {InstructionType::JP, AddressMode::D16, RegisterType::NONE, RegisterType::NONE, ConditionType::NZ};
    arr[0xC3] = {InstructionType::JP, AddressMode::D16};
    arr[0xC4] = {InstructionType::CALL, AddressMode::D16, RegisterType::NONE, RegisterType::NONE, ConditionType::NZ};
    arr[0xC5] = {InstructionType::PUSH, AddressMode::R, RegisterType::BC};
    arr[0xC7] = {InstructionType::RST, AddressMode::IMPLY, RegisterType::NONE, RegisterType::NONE, ConditionType::NONE, 0x00};
    arr[0xC8] = {InstructionType::RET, AddressMode::IMPLY, RegisterType::NONE, RegisterType::NONE, ConditionType::Z};
    arr[0xC9] = {InstructionType::RET};
    arr[0xCA] = {InstructionType::JP, AddressMode::D16, RegisterType::NONE, RegisterType::NONE, ConditionType::Z};
    arr[0xCC] = {InstructionType::CALL, AddressMode::D16, RegisterType::NONE, RegisterType::NONE, ConditionType::Z};
    arr[0xCD] = {InstructionType::CALL, AddressMode::D16};
    arr[0xCF] = {InstructionType::RST, AddressMode::IMPLY, RegisterType::NONE, RegisterType::NONE, ConditionType::NONE, 0x08};
    
    //0xDx
    arr[0xD0] = {InstructionType::RET, AddressMode::IMPLY, RegisterType::NONE, RegisterType::NONE, ConditionType::NC};
    arr[0xD1] = {InstructionType::POP, AddressMode::R, RegisterType::DE};
    arr[0xD2] = {InstructionType::JP, AddressMode::D16, RegisterType::NONE, RegisterType::NONE, ConditionType::NC};
    arr[0xD4] = {InstructionType::CALL, AddressMode::D16, RegisterType::NONE, RegisterType::NONE, ConditionType::NC};
    arr[0xD5] = {InstructionType::PUSH, AddressMode::R, RegisterType::DE};
    arr[0xD7] = {InstructionType::RST, AddressMode::IMPLY, RegisterType::NONE, RegisterType::NONE, ConditionType::NONE, 0x10};
    arr[0xD8] = {InstructionType::RET, AddressMode::IMPLY, RegisterType::NONE, RegisterType::NONE, ConditionType::C};
    arr[0xD9] = {InstructionType::RETI};
    arr[0xDA] = {InstructionType::JP, AddressMode::D16, RegisterType::NONE, RegisterType::NONE, ConditionType::C};
    arr[0xDC] = {InstructionType::CALL, AddressMode::D16, RegisterType::NONE, RegisterType::NONE, ConditionType::C};
    arr[0xDF] = {InstructionType::RST, AddressMode::IMPLY, RegisterType::NONE, RegisterType::NONE, ConditionType::NONE, 0x18};

    //0xEx
    arr[0xE0] = {InstructionType::LDH, AddressMode::A8_R, RegisterType::NONE, RegisterType::A};
    arr[0xE1] = {InstructionType::POP, AddressMode::R, RegisterType::HL};
    arr[0xE2] = {InstructionType::LD, AddressMode::MR_R, RegisterType::C, RegisterType::A};
    arr[0xE5] = {InstructionType::PUSH, AddressMode::R, RegisterType::HL};
    arr[0xE7] = {InstructionType::RST, AddressMode::IMPLY, RegisterType::NONE, RegisterType::NONE, ConditionType::NONE, 0x20};
    arr[0xE9] = {InstructionType::JP, AddressMode::MR, RegisterType::HL};
    arr[0xEA] = {InstructionType::LD, AddressMode::A16_R, RegisterType::NONE, RegisterType::A};
    arr[0xEF] = {InstructionType::RST, AddressMode::IMPLY, RegisterType::NONE, RegisterType::NONE, ConditionType::NONE, 0x28};

    //0xFx
    arr[0xF0] = {InstructionType::LDH, AddressMode::R_A8, RegisterType::A};
    arr[0xF1] = {InstructionType::POP, AddressMode::R, RegisterType::AF};
    arr[0xF2] = {InstructionType::LD, AddressMode::R_MR, RegisterType::A, RegisterType::C};
    arr[0xF3] = {InstructionType::DI};
    arr[0xF5] = {InstructionType::PUSH, AddressMode::R, RegisterType::AF};
    arr[0xF7] = {InstructionType::RST, AddressMode::IMPLY, RegisterType::NONE, RegisterType::NONE, ConditionType::NONE, 0x30};
    arr[0xFA] = {InstructionType::LD, AddressMode::R_A16, RegisterType::A};
    arr[0xFF] = {InstructionType::RST, AddressMode::IMPLY, RegisterType::NONE, RegisterType::NONE, ConditionType::NONE, 0x38};

    return arr;
}();