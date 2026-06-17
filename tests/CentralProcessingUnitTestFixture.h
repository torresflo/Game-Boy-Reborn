#pragma once

#include <initializer_list>

#include "CentralProcessingUnit.h"
#include "MemoryBus.h"

// Test-only helper that wires up a CentralProcessingUnit against a plain
// MemoryBus (no Cartridge needed, see MemoryBus.cpp) and exposes its private
// state via the CentralProcessingUnitTestFixture friendship.
//
// Note: emulateCycles() is currently a no-op stub, so per-instruction T-cycle
// counts cannot be asserted on yet. This fixture only verifies architectural
// correctness (registers/flags/memory).
class CentralProcessingUnitTestFixture
{
public:
    CentralProcessingUnitTestFixture()
    {
        cpu.initialize(&memoryBus);
    }

    void setRegister(RegisterType type, u16 value)
    {
        cpu.writeRegister(type, value);
    }

    u16 getRegister(RegisterType type) const
    {
        return cpu.readRegister(type);
    }

    void setFlags(bool zero, bool subtract, bool halfCarry, bool carry)
    {
        cpu.setFlagValues(zero, subtract, halfCarry, carry);
    }

    bool getFlagZero() const
    {
        return cpu.flagZ();
    }

    bool getFlagSubtract() const
    {
        return cpu.flagN();
    }

    bool getFlagHalfCarry() const
    {
        return cpu.flagH();
    }

    bool getFlagCarry() const
    {
        return cpu.flagC();
    }

    bool getInterruptMasterEnabled() const
    {
        return cpu.interruptMasterEnabled;
    }

    u8 readMemory(u16 address) const
    {
        return memoryBus.read(address);
    }

    void writeMemory(u16 address, u8 value)
    {
        memoryBus.write(address, value);
    }

    // Writes the bytes at startAddress (use the WRAM range, 0xC000-0xDFFF) and
    // points PC at them so the next step() executes this instruction.
    void loadProgram(u16 startAddress, std::initializer_list<u8> bytes)
    {
        u16 address = startAddress;
        for (u8 byte : bytes)
        {
            memoryBus.write(address, byte);
            address++;
        }

        cpu.registers.PC = startAddress;
    }

    void step()
    {
        cpu.step();
    }

private:
    MemoryBus memoryBus;
    CentralProcessingUnit cpu;
};
