#pragma once

#include <map>

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
    SET
};

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

const std::map<u8, Instruction> Instructions =
{
    {0x00, {InstructionType::NOP}},
    {0x05, {InstructionType::DEC, AdressMode::R, RegisterType::B}},
    {0x0E, {InstructionType::LD, AdressMode::R_D8, RegisterType::C}},
    {0xAF, {InstructionType::XOR, AdressMode::R, RegisterType::A}},
    {0xC3, {InstructionType::JP, AdressMode::D16}},
};