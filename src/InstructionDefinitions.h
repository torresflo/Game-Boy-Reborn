#pragma once

#include <array>

#include "Common.h"

enum class AdressMode
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
        case InstructionType::NONE:  return "NONE";
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

struct Instruction
{
    InstructionType type = InstructionType::NONE;
    AdressMode addressMode = AdressMode::IMPLY;
    RegisterType register1 = RegisterType::NONE;
    RegisterType register2 = RegisterType::NONE;
    ConditionType condition = ConditionType::NONE;
    u8 param = 0;
};

inline const std::array<Instruction, 256> Instructions = []() {
    std::array<Instruction, 256> arr{};
    arr[0x00] = {InstructionType::NOP};
    arr[0x05] = {InstructionType::DEC, AdressMode::R, RegisterType::B};
    arr[0x0E] = {InstructionType::LD, AdressMode::R_D8, RegisterType::C};
    arr[0xAF] = {InstructionType::XOR, AdressMode::R, RegisterType::A};
    arr[0xC3] = {InstructionType::JP, AdressMode::D16};
    arr[0xF3] = {InstructionType::DI};
    return arr;
}();