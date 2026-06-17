#include <doctest/doctest.h>

#include "CentralProcessingUnitTestFixture.h"

namespace
{
    constexpr u16 ProgramStart = 0xC000;
}

TEST_CASE("LD R,R copies one register into another (R_R)")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::B, 0x3C);

    fixture.loadProgram(ProgramStart, {0x78}); // LD A,B
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::A) == 0x3C);
}

TEST_CASE("LD R,d8 loads an immediate byte into a register (R_D8)")
{
    CentralProcessingUnitTestFixture fixture;

    fixture.loadProgram(ProgramStart, {0x06, 0x99}); // LD B,d8
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::B) == 0x99);
    CHECK(fixture.getRegister(RegisterType::PC) == ProgramStart + 2);
}

TEST_CASE("LD R,d16 loads an immediate word into a 16-bit register (R_D16)")
{
    CentralProcessingUnitTestFixture fixture;

    fixture.loadProgram(ProgramStart, {0x01, 0x34, 0x12}); // LD BC,d16
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::BC) == 0x1234);
}

TEST_CASE("LD (R),R stores a register into the address held by another register (MR_R)")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::BC, 0xC010);
    fixture.setRegister(RegisterType::A, 0x42);

    fixture.loadProgram(ProgramStart, {0x02}); // LD (BC),A
    fixture.step();

    CHECK(fixture.readMemory(0xC010) == 0x42);
}

TEST_CASE("LD R,(R) loads a register from the address held by another register (R_MR)")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::BC, 0xC020);
    fixture.writeMemory(0xC020, 0x77);

    fixture.loadProgram(ProgramStart, {0x0A}); // LD A,(BC)
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::A) == 0x77);
}

TEST_CASE("LD A,(HL+) loads from HL then increments it (R_HLI)")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::HL, 0xC030);
    fixture.writeMemory(0xC030, 0x99);

    fixture.loadProgram(ProgramStart, {0x2A}); // LD A,(HL+)
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::A) == 0x99);
    CHECK(fixture.getRegister(RegisterType::HL) == 0xC031);
}

TEST_CASE("LD A,(HL-) loads from HL then decrements it (R_HLD)")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::HL, 0xC030);
    fixture.writeMemory(0xC030, 0x88);

    fixture.loadProgram(ProgramStart, {0x3A}); // LD A,(HL-)
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::A) == 0x88);
    CHECK(fixture.getRegister(RegisterType::HL) == 0xC02F);
}

TEST_CASE("LD (HL+),A stores A then increments HL (HLI_R)")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::HL, 0xC040);
    fixture.setRegister(RegisterType::A, 0xAB);

    fixture.loadProgram(ProgramStart, {0x22}); // LD (HL+),A
    fixture.step();

    CHECK(fixture.readMemory(0xC040) == 0xAB);
    CHECK(fixture.getRegister(RegisterType::HL) == 0xC041);
}

TEST_CASE("LD (HL-),A stores A then decrements HL (HLD_R)")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::HL, 0xC040);
    fixture.setRegister(RegisterType::A, 0xCD);

    fixture.loadProgram(ProgramStart, {0x32}); // LD (HL-),A
    fixture.step();

    CHECK(fixture.readMemory(0xC040) == 0xCD);
    CHECK(fixture.getRegister(RegisterType::HL) == 0xC03F);
}

TEST_CASE("LD (HL),d8 stores an immediate byte at the address held by HL (MR_D8)")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::HL, 0xC050);

    fixture.loadProgram(ProgramStart, {0x36, 0x77}); // LD (HL),d8
    fixture.step();

    CHECK(fixture.readMemory(0xC050) == 0x77);
}

TEST_CASE("LD (a16),A stores A at an immediate 16-bit address (A16_R, 8-bit source)")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::A, 0xCD);

    fixture.loadProgram(ProgramStart, {0xEA, 0x10, 0xC0}); // LD (0xC010),A
    fixture.step();

    CHECK(fixture.readMemory(0xC010) == 0xCD);
}

TEST_CASE("LD (a16),SP stores SP at an immediate 16-bit address (A16_R, 16-bit source)")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::SP, 0xBEEF);

    fixture.loadProgram(ProgramStart, {0x08, 0x20, 0xC0}); // LD (0xC020),SP
    fixture.step();

    CHECK(fixture.readMemory(0xC020) == 0xEF); // low byte
    CHECK(fixture.readMemory(0xC021) == 0xBE); // high byte
}

TEST_CASE("LD A,(a16) loads A from an immediate 16-bit address (R_A16)")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.writeMemory(0xC060, 0x99);

    fixture.loadProgram(ProgramStart, {0xFA, 0x60, 0xC0}); // LD A,(0xC060)
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::A) == 0x99);
}

TEST_CASE("LDH (a8),A stores A in the high page (A8_R)")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::A, 0x55);

    fixture.loadProgram(ProgramStart, {0xE0, 0x80}); // LDH (0xFF80),A
    fixture.step();

    CHECK(fixture.readMemory(0xFF80) == 0x55);
}

TEST_CASE("LDH A,(a8) loads A from the high page (R_A8)")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.writeMemory(0xFF80, 0x66);

    fixture.loadProgram(ProgramStart, {0xF0, 0x80}); // LDH A,(0xFF80)
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::A) == 0x66);
}
