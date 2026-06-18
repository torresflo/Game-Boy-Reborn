#include <doctest/doctest.h>

#include "CentralProcessingUnitTestFixture.h"

namespace
{
    constexpr u16 ProgramStart = 0xC000;
}

TEST_CASE("AND A,R performs a bitwise and and always sets the half-carry flag")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("typical value")
    {
        fixture.setRegister(RegisterType::A, 0xF0);
        fixture.setRegister(RegisterType::B, 0x3C);
        fixture.setFlags(false, true, false, true);

        fixture.loadProgram(ProgramStart, {0xA0}); // AND A,B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::A) == 0x30);
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagSubtract() == false);
        CHECK(fixture.getFlagHalfCarry() == true);
        CHECK(fixture.getFlagCarry() == false);
    }

    SUBCASE("zero result sets the zero flag")
    {
        fixture.setRegister(RegisterType::A, 0xF0);
        fixture.setRegister(RegisterType::B, 0x0F);

        fixture.loadProgram(ProgramStart, {0xA0}); // AND A,B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::A) == 0x00);
        CHECK(fixture.getFlagZero() == true);
        CHECK(fixture.getFlagHalfCarry() == true);
        CHECK(fixture.getFlagCarry() == false);
    }
}

TEST_CASE("AND A,d8 performs a bitwise and with an immediate byte")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::A, 0xFF);

    fixture.loadProgram(ProgramStart, {0xE6, 0x0F}); // AND A,0x0F
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::A) == 0x0F);
    CHECK(fixture.getFlagZero() == false);
    CHECK(fixture.getFlagHalfCarry() == true);
    CHECK(fixture.getFlagCarry() == false);
}

TEST_CASE("OR A,R performs a bitwise or and clears subtract, half-carry and carry")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("typical value")
    {
        fixture.setRegister(RegisterType::A, 0xF0);
        fixture.setRegister(RegisterType::B, 0x0F);
        fixture.setFlags(false, true, true, true);

        fixture.loadProgram(ProgramStart, {0xB0}); // OR A,B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::A) == 0xFF);
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagSubtract() == false);
        CHECK(fixture.getFlagHalfCarry() == false);
        CHECK(fixture.getFlagCarry() == false);
    }

    SUBCASE("zero result sets the zero flag")
    {
        fixture.setRegister(RegisterType::A, 0x00);
        fixture.setRegister(RegisterType::B, 0x00);

        fixture.loadProgram(ProgramStart, {0xB0}); // OR A,B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::A) == 0x00);
        CHECK(fixture.getFlagZero() == true);
    }
}

TEST_CASE("OR A,d8 performs a bitwise or with an immediate byte")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::A, 0x00);

    fixture.loadProgram(ProgramStart, {0xF6, 0x5A}); // OR A,0x5A
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::A) == 0x5A);
    CHECK(fixture.getFlagZero() == false);
}

TEST_CASE("XOR A,A zeroes the accumulator and sets the zero flag")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::A, 0x5A);
    fixture.setFlags(false, true, true, true);

    fixture.loadProgram(ProgramStart, {0xAF}); // XOR A,A
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::A) == 0x00);
    CHECK(fixture.getFlagZero() == true);
    CHECK(fixture.getFlagSubtract() == false);
    CHECK(fixture.getFlagHalfCarry() == false);
    CHECK(fixture.getFlagCarry() == false);
}

TEST_CASE("XOR A,R performs a bitwise exclusive or")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::A, 0xF0);
    fixture.setRegister(RegisterType::B, 0xFF);

    fixture.loadProgram(ProgramStart, {0xA8}); // XOR A,B
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::A) == 0x0F);
    CHECK(fixture.getFlagZero() == false);
}

TEST_CASE("XOR A,d8 performs a bitwise exclusive or with an immediate byte")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::A, 0xFF);

    fixture.loadProgram(ProgramStart, {0xEE, 0xFF}); // XOR A,0xFF
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::A) == 0x00);
    CHECK(fixture.getFlagZero() == true);
}

TEST_CASE("CP A,R compares without modifying A and updates flags like a subtraction")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("equal operands set the zero flag")
    {
        fixture.setRegister(RegisterType::A, 0x10);
        fixture.setRegister(RegisterType::B, 0x10);

        fixture.loadProgram(ProgramStart, {0xB8}); // CP A,B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::A) == 0x10); // A is unchanged
        CHECK(fixture.getFlagZero() == true);
        CHECK(fixture.getFlagSubtract() == true);
        CHECK(fixture.getFlagHalfCarry() == false);
        CHECK(fixture.getFlagCarry() == false);
    }

    SUBCASE("borrow sets the carry flag without modifying A")
    {
        fixture.setRegister(RegisterType::A, 0x01);
        fixture.setRegister(RegisterType::B, 0x02);

        fixture.loadProgram(ProgramStart, {0xB8}); // CP A,B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::A) == 0x01); // A is unchanged
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagSubtract() == true);
        CHECK(fixture.getFlagHalfCarry() == true);
        CHECK(fixture.getFlagCarry() == true);
    }
}

TEST_CASE("CP A,d8 compares A against an immediate byte")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::A, 0x3C);

    fixture.loadProgram(ProgramStart, {0xFE, 0x3C}); // CP A,0x3C
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::A) == 0x3C);
    CHECK(fixture.getFlagZero() == true);
    CHECK(fixture.getFlagSubtract() == true);
}
