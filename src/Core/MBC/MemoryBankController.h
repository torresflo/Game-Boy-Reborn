#pragma once

#include <vector>

#include "Common.h"
#include "Save/ISaveStateSerializable.h"

class Cartridge;

// Cartridge-side interface for Memory Bank Controller chips. Implementations
// only ever see the two address ranges the cartridge edge connector exposes:
// 0x0000-0x7FFF (ROM and bank-control registers) and 0xA000-0xBFFF (external RAM).
class MemoryBankController : public ISaveStateSerializable
{
public:
    MemoryBankController(Cartridge* cartridgePtr, bool hasBatteryFlag);
    virtual ~MemoryBankController() = default;

    virtual u8 read(u16 address) const = 0;
    virtual void write(u16 address, u8 value) = 0;

    // Called once per T-cycle. Only chips with extra timing-sensitive
    // hardware (eg. MBC3's RTC) need to override this.
    virtual void tick();

    // Default no-op, sufficient for chips with no extra banking/RTC state (eg. NoMemoryBankController).
    virtual void serialize(SaveStateWriter& writer) const override;
    virtual void deserialize(SaveStateReader& reader) override;

    bool hasBattery() const;

protected:
    const std::vector<u8>& getROMData() const;
    std::vector<u8>& getRAMData();
    const std::vector<u8>& getRAMData() const;

    Cartridge* cartridge = nullptr;

private:
    bool batteryPresent = false;
};
