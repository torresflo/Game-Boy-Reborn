#include <doctest/doctest.h>

#include "CentralProcessingUnitTestFixture.h"

namespace
{
    constexpr u16 ProgramStart = 0xC000;
}

TEST_CASE("RLC R rotates left circularly")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("bit 7 set wraps into carry and bit 0")
    {
        fixture.setRegister(RegisterType::B, 0x85); // 1000 0101

        fixture.loadProgram(ProgramStart, {0xCB, 0x00}); // RLC B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::B) == 0x0B);
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagSubtract() == false);
        CHECK(fixture.getFlagHalfCarry() == false);
        CHECK(fixture.getFlagCarry() == true);
    }

    SUBCASE("zero result sets the zero flag")
    {
        fixture.setRegister(RegisterType::B, 0x00);

        fixture.loadProgram(ProgramStart, {0xCB, 0x00}); // RLC B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::B) == 0x00);
        CHECK(fixture.getFlagZero() == true);
        CHECK(fixture.getFlagCarry() == false);
    }
}

TEST_CASE("RLC (HL) operates on the byte pointed to by HL")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::HL, 0xC070);
    fixture.writeMemory(0xC070, 0x85);

    fixture.loadProgram(ProgramStart, {0xCB, 0x06}); // RLC (HL)
    fixture.step();

    CHECK(fixture.readMemory(0xC070) == 0x0B);
    CHECK(fixture.getFlagCarry() == true);
}

TEST_CASE("RRC R rotates right circularly")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::B, 0x01);

    fixture.loadProgram(ProgramStart, {0xCB, 0x08}); // RRC B
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::B) == 0x80);
    CHECK(fixture.getFlagZero() == false);
    CHECK(fixture.getFlagSubtract() == false);
    CHECK(fixture.getFlagHalfCarry() == false);
    CHECK(fixture.getFlagCarry() == true);
}

TEST_CASE("RL R rotates left through the carry flag")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("carry-in feeds bit 0, bit 7 feeds carry-out")
    {
        fixture.setRegister(RegisterType::B, 0x80);
        fixture.setFlags(false, false, false, true);

        fixture.loadProgram(ProgramStart, {0xCB, 0x10}); // RL B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::B) == 0x01);
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagCarry() == true);
    }

    SUBCASE("no carry-in and no carry-out")
    {
        fixture.setRegister(RegisterType::B, 0x01);
        fixture.setFlags(false, false, false, false);

        fixture.loadProgram(ProgramStart, {0xCB, 0x10}); // RL B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::B) == 0x02);
        CHECK(fixture.getFlagCarry() == false);
    }
}

TEST_CASE("RR R rotates right through the carry flag")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::B, 0x01);
    fixture.setFlags(false, false, false, true);

    fixture.loadProgram(ProgramStart, {0xCB, 0x18}); // RR B
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::B) == 0x80);
    CHECK(fixture.getFlagZero() == false);
    CHECK(fixture.getFlagCarry() == true);
}

TEST_CASE("SLA R shifts left, discarding bit 7 into carry")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("bit 7 set produces zero and carry")
    {
        fixture.setRegister(RegisterType::B, 0x80);

        fixture.loadProgram(ProgramStart, {0xCB, 0x20}); // SLA B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::B) == 0x00);
        CHECK(fixture.getFlagZero() == true);
        CHECK(fixture.getFlagSubtract() == false);
        CHECK(fixture.getFlagHalfCarry() == false);
        CHECK(fixture.getFlagCarry() == true);
    }

    SUBCASE("bit 7 clear leaves carry unset")
    {
        fixture.setRegister(RegisterType::B, 0x40);

        fixture.loadProgram(ProgramStart, {0xCB, 0x20}); // SLA B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::B) == 0x80);
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagCarry() == false);
    }
}

TEST_CASE("SRA R shifts right preserving the sign bit")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("negative value keeps bit 7 set")
    {
        fixture.setRegister(RegisterType::B, 0x81); // 1000 0001

        fixture.loadProgram(ProgramStart, {0xCB, 0x28}); // SRA B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::B) == 0xC0);
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagCarry() == true);
    }

    SUBCASE("positive value clears bit 7")
    {
        fixture.setRegister(RegisterType::B, 0x7F); // 0111 1111

        fixture.loadProgram(ProgramStart, {0xCB, 0x28}); // SRA B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::B) == 0x3F);
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagCarry() == true);
    }
}

TEST_CASE("SWAP R exchanges the high and low nibbles")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("typical value")
    {
        fixture.setRegister(RegisterType::B, 0xAB);
        fixture.setFlags(false, true, true, true);

        fixture.loadProgram(ProgramStart, {0xCB, 0x30}); // SWAP B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::B) == 0xBA);
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagSubtract() == false);
        CHECK(fixture.getFlagHalfCarry() == false);
        CHECK(fixture.getFlagCarry() == false);
    }

    SUBCASE("zero value sets the zero flag")
    {
        fixture.setRegister(RegisterType::B, 0x00);

        fixture.loadProgram(ProgramStart, {0xCB, 0x30}); // SWAP B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::B) == 0x00);
        CHECK(fixture.getFlagZero() == true);
    }
}

TEST_CASE("SRL R shifts right, discarding bit 0 into carry")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("bit 0 set produces carry")
    {
        fixture.setRegister(RegisterType::B, 0x01);

        fixture.loadProgram(ProgramStart, {0xCB, 0x38}); // SRL B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::B) == 0x00);
        CHECK(fixture.getFlagZero() == true);
        CHECK(fixture.getFlagSubtract() == false);
        CHECK(fixture.getFlagHalfCarry() == false);
        CHECK(fixture.getFlagCarry() == true);
    }

    SUBCASE("bit 0 clear leaves carry unset")
    {
        fixture.setRegister(RegisterType::B, 0x80);

        fixture.loadProgram(ProgramStart, {0xCB, 0x38}); // SRL B
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::B) == 0x40);
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagCarry() == false);
    }
}

TEST_CASE("BIT b,R tests a single bit without modifying the register")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("bit set clears the zero flag")
    {
        fixture.setRegister(RegisterType::A, 0x80);
        fixture.setFlags(false, false, false, true);

        fixture.loadProgram(ProgramStart, {0xCB, 0x7F}); // BIT 7,A
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::A) == 0x80);
        CHECK(fixture.getFlagZero() == false);
        CHECK(fixture.getFlagSubtract() == false);
        CHECK(fixture.getFlagHalfCarry() == true);
        CHECK(fixture.getFlagCarry() == true); // untouched
    }

    SUBCASE("bit clear sets the zero flag")
    {
        fixture.setRegister(RegisterType::A, 0x7F);

        fixture.loadProgram(ProgramStart, {0xCB, 0x7F}); // BIT 7,A
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::A) == 0x7F);
        CHECK(fixture.getFlagZero() == true);
        CHECK(fixture.getFlagHalfCarry() == true);
    }
}

TEST_CASE("BIT b,(HL) tests a bit of the byte pointed to by HL")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::HL, 0xC070);
    fixture.writeMemory(0xC070, 0x00);

    fixture.loadProgram(ProgramStart, {0xCB, 0x7E}); // BIT 7,(HL)
    fixture.step();

    CHECK(fixture.readMemory(0xC070) == 0x00);
    CHECK(fixture.getFlagZero() == true);
    CHECK(fixture.getFlagHalfCarry() == true);
}

TEST_CASE("RES b,R clears a single bit without touching flags")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::B, 0xFF);
    fixture.setFlags(true, true, true, true);

    fixture.loadProgram(ProgramStart, {0xCB, 0x80}); // RES 0,B
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::B) == 0xFE);
    CHECK(fixture.getFlagZero() == true);
    CHECK(fixture.getFlagSubtract() == true);
    CHECK(fixture.getFlagHalfCarry() == true);
    CHECK(fixture.getFlagCarry() == true);
}

TEST_CASE("RES b,(HL) clears a single bit of the byte pointed to by HL")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::HL, 0xC070);
    fixture.writeMemory(0xC070, 0xFF);

    fixture.loadProgram(ProgramStart, {0xCB, 0x86}); // RES 0,(HL)
    fixture.step();

    CHECK(fixture.readMemory(0xC070) == 0xFE);
}

TEST_CASE("SET b,R sets a single bit without touching flags")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::B, 0x00);
    fixture.setFlags(true, true, true, true);

    fixture.loadProgram(ProgramStart, {0xCB, 0xC0}); // SET 0,B
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::B) == 0x01);
    CHECK(fixture.getFlagZero() == true);
    CHECK(fixture.getFlagSubtract() == true);
    CHECK(fixture.getFlagHalfCarry() == true);
    CHECK(fixture.getFlagCarry() == true);
}

TEST_CASE("SET b,(HL) sets a single bit of the byte pointed to by HL")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::HL, 0xC070);
    fixture.writeMemory(0xC070, 0x00);

    fixture.loadProgram(ProgramStart, {0xCB, 0xC6}); // SET 0,(HL)
    fixture.step();

    CHECK(fixture.readMemory(0xC070) == 0x01);
}
