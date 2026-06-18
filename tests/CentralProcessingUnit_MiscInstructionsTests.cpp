#include <doctest/doctest.h>

#include "CentralProcessingUnitTestFixture.h"

namespace
{
    constexpr u16 ProgramStart = 0xC000;
}

TEST_CASE("NOP advances PC without touching registers or flags")
{
    CentralProcessingUnitTestFixture fixture;
    fixture.setRegister(RegisterType::A, 0x42);
    fixture.setFlags(true, false, true, false);

    fixture.loadProgram(ProgramStart, {0x00});
    fixture.step();

    CHECK(fixture.getRegister(RegisterType::PC) == ProgramStart + 1);
    CHECK(fixture.getRegister(RegisterType::A) == 0x42);
    CHECK(fixture.getFlagZero() == true);
    CHECK(fixture.getFlagSubtract() == false);
    CHECK(fixture.getFlagHalfCarry() == true);
    CHECK(fixture.getFlagCarry() == false);
}

TEST_CASE("DI disables the interrupt master enable flag")
{
    CentralProcessingUnitTestFixture fixture;
    REQUIRE(fixture.getInterruptMasterEnabled() == true);

    fixture.loadProgram(ProgramStart, {0xF3});
    fixture.step();

    CHECK(fixture.getInterruptMasterEnabled() == false);
}
