#pragma once

#include <array>
#include <vector>

#include "Common.h"
#include "MemoryBus.h"

// Test-only MemoryBus backed by a flat 64KB array covering the entire address
// space, bypassing the real ROM/VRAM/OAM/echo/IO region routing entirely.
// External JSON CPU test vectors place opcodes and operands at random
// addresses across all 64KB, which the real MemoryBus can't faithfully store
// outside WRAM/HRAM/IE. Every access is also recorded so a single step()'s
// bus trace can be compared against a test vector's expected cycle log.
class FlatMemoryBus : public MemoryBus
{
public:
    struct BusAccess
    {
        u16 address;
        u8 value;
        bool isWrite;
    };

    u8 read(u16 address) const override
    {
        u8 value = memory[address];
        accesses.push_back({address, value, false});
        return value;
    }

    void write(u16 address, u8 value) override
    {
        memory[address] = value;
        accesses.push_back({address, value, true});
    }

    void resetLog()
    {
        accesses.clear();
    }

    const std::vector<BusAccess>& accessLog() const
    {
        return accesses;
    }

private:
    std::array<u8, 0x10000> memory{};
    mutable std::vector<BusAccess> accesses;
};
