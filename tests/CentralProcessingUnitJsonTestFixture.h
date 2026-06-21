#pragma once

#include <vector>

#include "CentralProcessingUnit.h"
#include "FlatMemoryBus.h"
#include "PixelProcessingUnit.h"

// Test-only helper that wires up a CentralProcessingUnit against a
// FlatMemoryBus (full 64KB address space) so randomized external JSON CPU
// test vectors, which place register/opcode/operand data anywhere in
// memory, can be loaded and verified faithfully.
class CentralProcessingUnitJsonTestFixture
{
public:
    CentralProcessingUnitJsonTestFixture()
    {
        cpu.initialize(&memoryBus, &ppu);
        ppu.initialize(&memoryBus, &cpu);
    }

    void setRegister(RegisterType type, u16 value)
    {
        cpu.writeRegister(type, value);
    }

    u16 getRegister(RegisterType type) const
    {
        return cpu.readRegister(type);
    }

    void setInterruptMasterEnabled(bool enabled)
    {
        cpu.interruptMasterEnabled = enabled;
    }

    bool getInterruptMasterEnabled() const
    {
        return cpu.interruptMasterEnabled;
    }

    void setInterruptEnableRegister(u8 value)
    {
        memoryBus.write(0xFFFF, value);
    }

    u8 readMemory(u16 address) const
    {
        return memoryBus.read(address);
    }

    void writeMemory(u16 address, u8 value)
    {
        memoryBus.write(address, value);
    }

    void resetBusLog()
    {
        memoryBus.resetLog();
    }

    const std::vector<FlatMemoryBus::BusAccess>& busLog() const
    {
        return memoryBus.accessLog();
    }

    u64 cyclesElapsed() const
    {
        return cpu.cycles;
    }

    void step()
    {
        cpu.step();
    }

    // Fetches and executes one instruction only, skipping the trailing
    // interrupt dispatch that step() performs. Matches the SM83 JSON test
    // vectors' notion of a "step", which never folds interrupt dispatch in.
    void executeInstruction()
    {
        cpu.executeNextInstruction();
    }

private:
    FlatMemoryBus memoryBus;
    PixelProcessingUnit ppu;
    CentralProcessingUnit cpu;
};
