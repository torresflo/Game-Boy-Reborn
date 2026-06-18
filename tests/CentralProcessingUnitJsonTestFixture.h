#pragma once

#include <vector>

#include "CentralProcessingUnit.h"
#include "FlatMemoryBus.h"

// Test-only helper that wires up a CentralProcessingUnit against a
// FlatMemoryBus (full 64KB address space) so randomized external JSON CPU
// test vectors, which place register/opcode/operand data anywhere in
// memory, can be loaded and verified faithfully.
class CentralProcessingUnitJsonTestFixture
{
public:
    CentralProcessingUnitJsonTestFixture()
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
        cpu.interruptEnable = value;
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

private:
    FlatMemoryBus memoryBus;
    CentralProcessingUnit cpu;
};
