#include <doctest/doctest.h>

#include "CentralProcessingUnitTestFixture.h"

namespace
{
    constexpr u16 ProgramStart = 0xC000;
    constexpr u16 StackTop = 0xDFF0;
}

TEST_CASE("JP a16 jumps unconditionally to an immediate address")
{
    CentralProcessingUnitTestFixture fixture;

    fixture.loadProgram(ProgramStart, {0xC3, 0x34, 0x12}); // JP 0x1234
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::PC) == 0x1234);
}

TEST_CASE("JP NZ,a16 only jumps when the condition holds")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("condition true (Z clear) jumps")
    {
        fixture.setFlags(false, false, false, false);
        fixture.loadProgram(ProgramStart, {0xC2, 0x00, 0xC1}); // JP NZ,0xC100
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::PC) == 0xC100);
    }

    SUBCASE("condition false (Z set) falls through")
    {
        fixture.setFlags(true, false, false, false);
        fixture.loadProgram(ProgramStart, {0xC2, 0x00, 0xC1}); // JP NZ,0xC100
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::PC) == ProgramStart + 3);
    }
}

TEST_CASE("JP (HL) jumps to the address held in HL")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::HL, 0xC0AB);

    fixture.loadProgram(ProgramStart, {0xE9}); // JP (HL)
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::PC) == 0xC0AB);
}

TEST_CASE("JR r8 jumps relative to the address right after the instruction")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("positive offset")
    {
        fixture.loadProgram(ProgramStart, {0x18, 0x05}); // JR +5
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::PC) == ProgramStart + 2 + 5);
    }

    SUBCASE("negative offset jumps backwards")
    {
        fixture.loadProgram(ProgramStart, {0x18, 0xFE}); // JR -2 (back to itself)
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::PC) == ProgramStart);
    }
}

TEST_CASE("JR NZ,r8 only jumps when the condition holds")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("condition true (Z clear) jumps")
    {
        fixture.setFlags(false, false, false, false);
        fixture.loadProgram(ProgramStart, {0x20, 0x05}); // JR NZ,+5
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::PC) == ProgramStart + 2 + 5);
    }

    SUBCASE("condition false (Z set) falls through")
    {
        fixture.setFlags(true, false, false, false);
        fixture.loadProgram(ProgramStart, {0x20, 0x05}); // JR NZ,+5
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::PC) == ProgramStart + 2);
    }
}

TEST_CASE("CALL a16 pushes the return address and jumps")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::SP, StackTop);

    fixture.loadProgram(ProgramStart, {0xCD, 0x00, 0xD0}); // CALL 0xD000
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::PC) == 0xD000);
    CHECK(fixture.getRegister(RegisterType::SP) == StackTop - 2);
    CHECK(fixture.readMemory(StackTop - 2) == 0x03); // low byte of return address (0xC003)
    CHECK(fixture.readMemory(StackTop - 1) == 0xC0); // high byte of return address
}

TEST_CASE("RET pops the return address and jumps")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::SP, StackTop - 2);
    fixture.writeMemory(StackTop - 2, 0x34); // low byte
    fixture.writeMemory(StackTop - 1, 0x12); // high byte

    fixture.loadProgram(ProgramStart, {0xC9}); // RET
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::PC) == 0x1234);
    CHECK(fixture.getRegister(RegisterType::SP) == StackTop);
}

TEST_CASE("RET NZ only pops and jumps when the condition holds")
{
    CentralProcessingUnitTestFixture fixture;

    SUBCASE("condition true (Z clear) returns")
    {
        fixture.setFlags(false, false, false, false);
        fixture.setRegister(RegisterType::SP, StackTop - 2);
        fixture.writeMemory(StackTop - 2, 0x34);
        fixture.writeMemory(StackTop - 1, 0x12);

        fixture.loadProgram(ProgramStart, {0xC0}); // RET NZ
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::PC) == 0x1234);
        CHECK(fixture.getRegister(RegisterType::SP) == StackTop);
    }

    SUBCASE("condition false (Z set) falls through, stack untouched")
    {
        fixture.setFlags(true, false, false, false);
        fixture.setRegister(RegisterType::SP, StackTop - 2);

        fixture.loadProgram(ProgramStart, {0xC0}); // RET NZ
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::PC) == ProgramStart + 1);
        CHECK(fixture.getRegister(RegisterType::SP) == StackTop - 2);
    }
}

TEST_CASE("RETI returns and re-enables the interrupt master enable flag")
{
    CentralProcessingUnitTestFixture fixture;

    // First disable interrupts via DI so re-enabling is observable.
    fixture.loadProgram(ProgramStart, {0xF3}); // DI
    fixture.step();
    REQUIRE(fixture.getInterruptMasterEnabled() == false);

    fixture.setRegister(RegisterType::SP, StackTop - 2);
    fixture.writeMemory(StackTop - 2, 0x00); // low byte
    fixture.writeMemory(StackTop - 1, 0xD0); // high byte

    fixture.loadProgram(ProgramStart, {0xD9}); // RETI
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::PC) == 0xD000);
    CHECK(fixture.getRegister(RegisterType::SP) == StackTop);
    CHECK(fixture.getInterruptMasterEnabled() == true);
}

TEST_CASE("RST pushes the return address and jumps to the fixed vector")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::SP, StackTop);

    fixture.loadProgram(ProgramStart, {0xCF}); // RST 0x08
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::PC) == 0x0008);
    CHECK(fixture.getRegister(RegisterType::SP) == StackTop - 2);
    CHECK(fixture.readMemory(StackTop - 2) == 0x01); // low byte of return address (0xC001)
    CHECK(fixture.readMemory(StackTop - 1) == 0xC0); // high byte of return address
}

TEST_CASE("RST 0x00 jumps to its own vector, not 0x08's")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::SP, StackTop);

    fixture.loadProgram(ProgramStart, {0xC7}); // RST 0x00
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::PC) == 0x0000);
    CHECK(fixture.getRegister(RegisterType::SP) == StackTop - 2);
    CHECK(fixture.readMemory(StackTop - 2) == 0x01); // low byte of return address (0xC001)
    CHECK(fixture.readMemory(StackTop - 1) == 0xC0); // high byte of return address
}

TEST_CASE("CALL NZ,a16 only pushes and jumps when the condition holds")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::SP, StackTop);

    SUBCASE("condition true (Z clear) calls")
    {
        fixture.setFlags(false, false, false, false);
        fixture.loadProgram(ProgramStart, {0xC4, 0x00, 0xD0}); // CALL NZ,0xD000
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::PC) == 0xD000);
        CHECK(fixture.getRegister(RegisterType::SP) == StackTop - 2);
        CHECK(fixture.readMemory(StackTop - 2) == 0x03); // low byte of return address (0xC003)
        CHECK(fixture.readMemory(StackTop - 1) == 0xC0); // high byte of return address
    }

    SUBCASE("condition false (Z set) falls through, stack untouched")
    {
        fixture.setFlags(true, false, false, false);
        fixture.loadProgram(ProgramStart, {0xC4, 0x00, 0xD0}); // CALL NZ,0xD000
        fixture.step();

        CHECK(fixture.getRegister(RegisterType::PC) == ProgramStart + 3);
        CHECK(fixture.getRegister(RegisterType::SP) == StackTop);
    }
}

TEST_CASE("CALL then RET returns to the original return address with SP restored")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::SP, StackTop);

    fixture.loadProgram(ProgramStart, {0xCD, 0x00, 0xD0}); // CALL 0xD000
    fixture.step();
    REQUIRE(fixture.getRegister(RegisterType::PC) == 0xD000);
    REQUIRE(fixture.getRegister(RegisterType::SP) == StackTop - 2);

    fixture.loadProgram(0xD000, {0xC9}); // RET, run from the call target
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::PC) == ProgramStart + 3);
    CHECK(fixture.getRegister(RegisterType::SP) == StackTop);
}

TEST_CASE("RST then RET restores PC and SP via the pushed return address")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::SP, StackTop);

    fixture.loadProgram(ProgramStart, {0xCF}); // RST 0x08
    fixture.step();
    REQUIRE(fixture.getRegister(RegisterType::PC) == 0x0008);
    REQUIRE(fixture.getRegister(RegisterType::SP) == StackTop - 2);

    // RST jumps into ROM (the fixed vector address), which this fixture can't
    // place code in (MemoryBus needs a cartridge for the ROM range). Run RET
    // from WRAM instead, to confirm it correctly consumes the address RST
    // pushed onto the stack.
    fixture.loadProgram(0xC100, {0xC9}); // RET
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::PC) == ProgramStart + 1);
    CHECK(fixture.getRegister(RegisterType::SP) == StackTop);
}
