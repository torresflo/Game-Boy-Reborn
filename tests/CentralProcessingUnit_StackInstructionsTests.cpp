#include <doctest/doctest.h>

#include "CentralProcessingUnitTestFixture.h"

namespace
{
    constexpr u16 ProgramStart = 0xC000;
    constexpr u16 StackTop = 0xDFF0;
}

TEST_CASE("PUSH R pushes a 16-bit register onto the stack, high byte first")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::BC, 0x1234);
    fixture.setRegister(RegisterType::SP, StackTop);

    fixture.loadProgram(ProgramStart, {0xC5}); // PUSH BC
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::SP) == StackTop - 2);
    CHECK(fixture.readMemory(StackTop - 1) == 0x12); // high byte
    CHECK(fixture.readMemory(StackTop - 2) == 0x34); // low byte
}

TEST_CASE("POP R pops two bytes off the stack into a 16-bit register")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::SP, StackTop - 2);
    fixture.writeMemory(StackTop - 2, 0x34); // low byte
    fixture.writeMemory(StackTop - 1, 0x12); // high byte

    fixture.loadProgram(ProgramStart, {0xC1}); // POP BC
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::BC) == 0x1234);
    CHECK(fixture.getRegister(RegisterType::SP) == StackTop);
}

TEST_CASE("POP AF masks off the unused low nibble of F")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::SP, StackTop - 2);
    fixture.writeMemory(StackTop - 2, 0xFF); // low byte (F), low nibble must be masked
    fixture.writeMemory(StackTop - 1, 0xAB); // high byte (A)

    fixture.loadProgram(ProgramStart, {0xF1}); // POP AF
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::AF) == 0xABF0);
}

TEST_CASE("PUSH then POP round trips a register pair and restores SP")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::DE, 0xBEEF);
    fixture.setRegister(RegisterType::SP, StackTop);

    fixture.loadProgram(ProgramStart, {0xD5}); // PUSH DE
    fixture.step();
    REQUIRE(fixture.getRegister(RegisterType::SP) == StackTop - 2);

    fixture.setRegister(RegisterType::DE, 0x0000);

    fixture.loadProgram(ProgramStart, {0xD1}); // POP DE
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::DE) == 0xBEEF);
    CHECK(fixture.getRegister(RegisterType::SP) == StackTop);
}

TEST_CASE("PUSH AF then POP AF still masks the unused low nibble of F")
{
    CentralProcessingUnitTestFixture fixture;
    // No real instruction sets F's low nibble; force it here to simulate
    // stale bits and confirm a PUSH/POP AF round trip still normalizes them.
    fixture.setRegister(RegisterType::AF, 0xABFF);
    fixture.setRegister(RegisterType::SP, StackTop);

    fixture.loadProgram(ProgramStart, {0xF5}); // PUSH AF
    fixture.step();
    REQUIRE(fixture.getRegister(RegisterType::SP) == StackTop - 2);

    fixture.loadProgram(ProgramStart, {0xF1}); // POP AF
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::AF) == 0xABF0);
    CHECK(fixture.getRegister(RegisterType::SP) == StackTop);
}
