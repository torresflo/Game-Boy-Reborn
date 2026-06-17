#include <doctest/doctest.h>

#include "CentralProcessingUnitTestFixture.h"

namespace
{
    constexpr u16 ProgramStart = 0xC000;
}

TEST_CASE("INC R increments an 8-bit register and updates flags")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("typical value")
    {
        fixture.setRegister(RegisterType::B, 0x05);
        fixture.setFlags(false, false, false, true);

        fixture.loadProgram(ProgramStart, {0x04}); // INC B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::B) == 0x06);
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagSubtract() == false);
        CHECK(fixture.getFlagHalfCarry() == false);
        CHECK(fixture.getFlagCarry() == true); // INC never touches carry
    }

    SUBCASE("half-carry boundary 0x0F -> 0x10")
    {
        fixture.setRegister(RegisterType::B, 0x0F);

        fixture.loadProgram(ProgramStart, {0x04}); // INC B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::B) == 0x10);
        CHECK(fixture.getFlagHalfCarry() == true);
        CHECK(fixture.getFlagZero() == false);
    }

    SUBCASE("overflow 0xFF -> 0x00 sets zero and half-carry")
    {
        fixture.setRegister(RegisterType::B, 0xFF);

        fixture.loadProgram(ProgramStart, {0x04}); // INC B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::B) == 0x00);
        CHECK(fixture.getFlagZero() == true);
        CHECK(fixture.getFlagHalfCarry() == true);
    }
}

TEST_CASE("INC R on a 16-bit register wraps without touching flags")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::BC, 0x00FF);
    fixture.setFlags(true, true, true, true);

    fixture.loadProgram(ProgramStart, {0x03}); // INC BC
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::BC) == 0x0100);
    CHECK(fixture.getFlagZero() == true);
    CHECK(fixture.getFlagSubtract() == true);
    CHECK(fixture.getFlagHalfCarry() == true);
    CHECK(fixture.getFlagCarry() == true);
}

TEST_CASE("INC (HL) increments the byte pointed to by HL")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::HL, 0xC070);
    fixture.writeMemory(0xC070, 0x0F);

    fixture.loadProgram(ProgramStart, {0x34}); // INC (HL)
    fixture.step();

    CHECK(fixture.readMemory(0xC070) == 0x10);
    CHECK(fixture.getFlagHalfCarry() == true);
    CHECK(fixture.getFlagZero() == false);
}

TEST_CASE("DEC R decrements an 8-bit register and updates flags")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("down to zero")
    {
        fixture.setRegister(RegisterType::B, 0x01);

        fixture.loadProgram(ProgramStart, {0x05}); // DEC B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::B) == 0x00);
        CHECK(fixture.getFlagZero() == true);
        CHECK(fixture.getFlagSubtract() == true);
        CHECK(fixture.getFlagHalfCarry() == false);
    }

    SUBCASE("half-carry boundary 0x10 -> 0x0F")
    {
        fixture.setRegister(RegisterType::B, 0x10);

        fixture.loadProgram(ProgramStart, {0x05}); // DEC B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::B) == 0x0F);
        CHECK(fixture.getFlagHalfCarry() == true);
        CHECK(fixture.getFlagZero() == false);
    }
}

TEST_CASE("DEC R on a 16-bit register wraps without touching flags")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::BC, 0x0100);
    fixture.setFlags(true, false, true, false);

    fixture.loadProgram(ProgramStart, {0x0B}); // DEC BC
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::BC) == 0x00FF);
    CHECK(fixture.getFlagZero() == true);
    CHECK(fixture.getFlagSubtract() == false);
    CHECK(fixture.getFlagHalfCarry() == true);
    CHECK(fixture.getFlagCarry() == false);
}

TEST_CASE("DEC (HL) decrements the byte pointed to by HL")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::HL, 0xC070);
    fixture.writeMemory(0xC070, 0x10);

    fixture.loadProgram(ProgramStart, {0x35}); // DEC (HL)
    fixture.step();

    CHECK(fixture.readMemory(0xC070) == 0x0F);
    CHECK(fixture.getFlagHalfCarry() == true);
    CHECK(fixture.getFlagSubtract() == true);
}

TEST_CASE("ADD A,R adds an 8-bit register into A and updates flags")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("half-carry boundary")
    {
        fixture.setRegister(RegisterType::A, 0x0F);
        fixture.setRegister(RegisterType::B, 0x01);

        fixture.loadProgram(ProgramStart, {0x80}); // ADD A,B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::A) == 0x10);
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagSubtract() == false);
        CHECK(fixture.getFlagHalfCarry() == true);
        CHECK(fixture.getFlagCarry() == false);
    }

    SUBCASE("overflow sets zero, half-carry and carry")
    {
        fixture.setRegister(RegisterType::A, 0xFF);
        fixture.setRegister(RegisterType::B, 0x01);

        fixture.loadProgram(ProgramStart, {0x80}); // ADD A,B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::A) == 0x00);
        CHECK(fixture.getFlagZero() == true);
        CHECK(fixture.getFlagHalfCarry() == true);
        CHECK(fixture.getFlagCarry() == true);
    }
}

TEST_CASE("ADD HL,R adds a 16-bit register into HL, leaving zero flag untouched")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("half-carry boundary, no carry")
    {
        fixture.setRegister(RegisterType::HL, 0x0FFF);
        fixture.setRegister(RegisterType::BC, 0x0001);
        fixture.setFlags(true, true, false, false);

        fixture.loadProgram(ProgramStart, {0x09}); // ADD HL,BC
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::HL) == 0x1000);
        CHECK(fixture.getFlagZero() == true); // untouched
        CHECK(fixture.getFlagSubtract() == false);
        CHECK(fixture.getFlagHalfCarry() == true);
        CHECK(fixture.getFlagCarry() == false);
    }

    SUBCASE("wraps to zero and sets carry")
    {
        fixture.setRegister(RegisterType::HL, 0xFFFF);
        fixture.setRegister(RegisterType::BC, 0x0001);

        fixture.loadProgram(ProgramStart, {0x09}); // ADD HL,BC
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::HL) == 0x0000);
        CHECK(fixture.getFlagHalfCarry() == true);
        CHECK(fixture.getFlagCarry() == true);
    }
}

TEST_CASE("ADD SP,r8 computes flags from unsigned byte addition of the low byte")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("positive offset")
    {
        fixture.setRegister(RegisterType::SP, 0x0005);

        fixture.loadProgram(ProgramStart, {0xE8, 0x03}); // ADD SP,3
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::SP) == 0x0008);
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagSubtract() == false);
        CHECK(fixture.getFlagHalfCarry() == false);
        CHECK(fixture.getFlagCarry() == false);
    }

    SUBCASE("negative offset still reports carry from the unsigned byte addition")
    {
        fixture.setRegister(RegisterType::SP, 0x0010);

        fixture.loadProgram(ProgramStart, {0xE8, 0xFF}); // ADD SP,-1
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::SP) == 0x000F);
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagHalfCarry() == false);
        CHECK(fixture.getFlagCarry() == true);
    }
}

TEST_CASE("ADC A,R adds with carry-in and updates flags")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("carry-in folded into the half-carry boundary")
    {
        fixture.setRegister(RegisterType::A, 0x0E);
        fixture.setRegister(RegisterType::B, 0x01);
        fixture.setFlags(false, false, false, true);

        fixture.loadProgram(ProgramStart, {0x88}); // ADC A,B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::A) == 0x10);
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagHalfCarry() == true);
        CHECK(fixture.getFlagCarry() == false);
    }

    SUBCASE("carry-in pushes the result to wrap and sets carry-out")
    {
        fixture.setRegister(RegisterType::A, 0xFF);
        fixture.setRegister(RegisterType::B, 0x00);
        fixture.setFlags(false, false, false, true);

        fixture.loadProgram(ProgramStart, {0x88}); // ADC A,B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::A) == 0x00);
        CHECK(fixture.getFlagZero() == true);
        CHECK(fixture.getFlagHalfCarry() == true);
        CHECK(fixture.getFlagCarry() == true);
    }
}

TEST_CASE("ADC A,d8 adds an immediate byte with carry-in")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::A, 0x05);
    fixture.setFlags(false, false, false, false);

    fixture.loadProgram(ProgramStart, {0xCE, 0x03}); // ADC A,3
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::A) == 0x08);
    CHECK(fixture.getFlagZero() == false);
    CHECK(fixture.getFlagHalfCarry() == false);
    CHECK(fixture.getFlagCarry() == false);
}

TEST_CASE("SUB A,R subtracts an 8-bit register from A and updates flags")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("no borrow")
    {
        fixture.setRegister(RegisterType::A, 0x10);
        fixture.setRegister(RegisterType::B, 0x01);

        fixture.loadProgram(ProgramStart, {0x90}); // SUB A,B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::A) == 0x0F);
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagSubtract() == true);
        CHECK(fixture.getFlagHalfCarry() == true);
        CHECK(fixture.getFlagCarry() == false);
    }

    SUBCASE("full borrow wraps and sets carry")
    {
        fixture.setRegister(RegisterType::A, 0x01);
        fixture.setRegister(RegisterType::B, 0x02);

        fixture.loadProgram(ProgramStart, {0x90}); // SUB A,B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::A) == 0xFF);
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagHalfCarry() == true);
        CHECK(fixture.getFlagCarry() == true);
    }

    SUBCASE("equal operands give zero")
    {
        fixture.setRegister(RegisterType::A, 0x05);
        fixture.setRegister(RegisterType::B, 0x05);

        fixture.loadProgram(ProgramStart, {0x90}); // SUB A,B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::A) == 0x00);
        CHECK(fixture.getFlagZero() == true);
        CHECK(fixture.getFlagHalfCarry() == false);
        CHECK(fixture.getFlagCarry() == false);
    }
}

TEST_CASE("SBC A,R subtracts with carry-in and updates flags")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("typical borrow with carry-in")
    {
        fixture.setRegister(RegisterType::A, 0x10);
        fixture.setRegister(RegisterType::B, 0x01);
        fixture.setFlags(false, false, false, true);

        fixture.loadProgram(ProgramStart, {0x98}); // SBC A,B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::A) == 0x0E);
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagHalfCarry() == true);
        CHECK(fixture.getFlagCarry() == false);
    }

    SUBCASE("carry-in pushes a full borrow")
    {
        fixture.setRegister(RegisterType::A, 0x00);
        fixture.setRegister(RegisterType::B, 0x01);
        fixture.setFlags(false, false, false, true);

        fixture.loadProgram(ProgramStart, {0x98}); // SBC A,B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::A) == 0xFE);
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagHalfCarry() == true);
        CHECK(fixture.getFlagCarry() == true);
    }
}

TEST_CASE("ADD A,A with A=0x80 sets carry without half-carry")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::A, 0x80);

    fixture.loadProgram(ProgramStart, {0x87}); // ADD A,A
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::A) == 0x00);
    CHECK(fixture.getFlagZero() == true);
    CHECK(fixture.getFlagHalfCarry() == false);
    CHECK(fixture.getFlagCarry() == true);
}

TEST_CASE("SUB A,B with A=0x00,B=0x10 sets carry without half-carry")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::A, 0x00);
    fixture.setRegister(RegisterType::B, 0x10);

    fixture.loadProgram(ProgramStart, {0x90}); // SUB A,B
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::A) == 0xF0);
    CHECK(fixture.getFlagZero() == false);
    CHECK(fixture.getFlagHalfCarry() == false);
    CHECK(fixture.getFlagCarry() == true);
}

TEST_CASE("ADD HL,HL doubles HL and can set carry")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::HL, 0x8000);

    fixture.loadProgram(ProgramStart, {0x29}); // ADD HL,HL
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::HL) == 0x0000);
    CHECK(fixture.getFlagHalfCarry() == false);
    CHECK(fixture.getFlagCarry() == true);
}

TEST_CASE("ADD HL,SP adds the stack pointer into HL")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::HL, 0x0001);
    fixture.setRegister(RegisterType::SP, 0xFFFF);

    fixture.loadProgram(ProgramStart, {0x39}); // ADD HL,SP
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::HL) == 0x0000);
    CHECK(fixture.getFlagHalfCarry() == true);
    CHECK(fixture.getFlagCarry() == true);
}

TEST_CASE("INC SP and DEC SP wrap without touching flags")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("INC SP wraps from 0xFFFF to 0x0000")
    {
        fixture.setRegister(RegisterType::SP, 0xFFFF);
        fixture.setFlags(true, true, true, true);

        fixture.loadProgram(ProgramStart, {0x33}); // INC SP
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::SP) == 0x0000);
        CHECK(fixture.getFlagZero() == true);
        CHECK(fixture.getFlagSubtract() == true);
        CHECK(fixture.getFlagHalfCarry() == true);
        CHECK(fixture.getFlagCarry() == true);
    }

    SUBCASE("DEC SP wraps from 0x0000 to 0xFFFF")
    {
        fixture.setRegister(RegisterType::SP, 0x0000);
        fixture.setFlags(false, false, false, false);

        fixture.loadProgram(ProgramStart, {0x3B}); // DEC SP
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::SP) == 0xFFFF);
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagSubtract() == false);
        CHECK(fixture.getFlagHalfCarry() == false);
        CHECK(fixture.getFlagCarry() == false);
    }
}

TEST_CASE("INC (HL) wraps from 0xFF to 0x00")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::HL, 0xC070);
    fixture.writeMemory(0xC070, 0xFF);

    fixture.loadProgram(ProgramStart, {0x34}); // INC (HL)
    fixture.step();

    CHECK(fixture.readMemory(0xC070) == 0x00);
    CHECK(fixture.getFlagZero() == true);
    CHECK(fixture.getFlagHalfCarry() == true);
}

TEST_CASE("DEC (HL) wraps from 0x00 to 0xFF")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::HL, 0xC070);
    fixture.writeMemory(0xC070, 0x00);

    fixture.loadProgram(ProgramStart, {0x35}); // DEC (HL)
    fixture.step();

    CHECK(fixture.readMemory(0xC070) == 0xFF);
    CHECK(fixture.getFlagZero() == false);
    CHECK(fixture.getFlagHalfCarry() == true);
}
