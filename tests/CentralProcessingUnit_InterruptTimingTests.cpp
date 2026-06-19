#include <doctest/doctest.h>

#include "CentralProcessingUnitJsonTestFixture.h"

// Cross-instruction interrupt timing isn't covered by the SM83 JSON vectors
// (those only check the effect of a single opcode), so these tests cover it
// directly using the full step() (fetch+execute+interrupt dispatch).
TEST_CASE("EI delays IME enable by one instruction")
{
    CentralProcessingUnitJsonTestFixture fixture;

    fixture.setInterruptMasterEnabled(false);
    fixture.setInterruptEnableRegister(0xFF); // IE: all enabled
    fixture.writeMemory(0xFF0F, 0x01); // IF: VBlank pending

    fixture.writeMemory(0x0100, 0xFB); // EI
    fixture.writeMemory(0x0101, 0x00); // NOP
    fixture.writeMemory(0x0102, 0x00); // NOP

    fixture.step(); // execute EI
    CHECK(fixture.getInterruptMasterEnabled() == false);
    CHECK(fixture.getRegister(RegisterType::PC) == 0x0101);

    fixture.step(); // execute the NOP right after EI
    // Interrupt should have been serviced at the end of THIS step, after the
    // NOP executed, jumping to the VBlank vector instead of falling through
    // to PC 0x0102. Servicing it also clears IME again.
    CHECK(fixture.getRegister(RegisterType::PC) == 0x0040);
    CHECK(fixture.getInterruptMasterEnabled() == false);
}

TEST_CASE("EI immediately followed by DI never actually enables interrupts")
{
    CentralProcessingUnitJsonTestFixture fixture;

    fixture.setInterruptMasterEnabled(false);
    fixture.setInterruptEnableRegister(0xFF);
    fixture.writeMemory(0xFF0F, 0x01);

    fixture.writeMemory(0x0100, 0xFB); // EI
    fixture.writeMemory(0x0101, 0xF3); // DI
    fixture.writeMemory(0x0102, 0x00); // NOP

    fixture.step(); // EI
    fixture.step(); // DI - should immediately cancel the pending enable

    CHECK(fixture.getInterruptMasterEnabled() == false);
    CHECK(fixture.getRegister(RegisterType::PC) == 0x0102); // no interrupt dispatch
}
