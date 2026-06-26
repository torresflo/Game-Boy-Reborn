#pragma once

#include "Common.h"
#include "MemoryBankController.h"

// MBC3: supports up to 2MByte ROM and/or 32KByte RAM, plus an optional
// real-time clock (RTC) on TIMER variants. The RTC is cycle-driven via
// tick(), not host wall-clock time, so its behavior stays deterministic.
class MBC3 : public MemoryBankController
{
public:
    MBC3(Cartridge* cartridgePtr, u32 romBankCount, u32 ramBankCount, bool hasBattery, bool hasTimer);

    u8 read(u16 address) const override;
    void write(u16 address, u8 value) override;
    void tick() override;

    virtual void serialize(SaveStateWriter& writer) const override;
    virtual void deserialize(SaveStateReader& reader) override;

private:
    static constexpr u32 ROMBankSizeBytes = 0x4000;
    static constexpr u32 RAMBankSizeBytes = 0x2000;
    static constexpr u32 CyclesPerSecond = 4194304; // Matches GameBoyEmulator::ClockFrequencyHz

    static constexpr u8 RTCRegisterSelectMin = 0x08;
    static constexpr u8 RTCRegisterSelectMax = 0x0C;

    bool isRTCRegisterSelected() const;
    void advanceRTCBySecond();
    void latchRTC();

    u32 ROMBankCount;
    u32 RAMBankCount;
    bool timerPresent;

    u8 romBankNumber = 1; // 7-bit register, 0x2000-0x3FFF. Writing 0 is treated as 1.
    u8 ramOrRTCSelect = 0; // 0x4000-0x5FFF: 0x00-0x03 = RAM bank, 0x08-0x0C = RTC register
    bool ramAndTimerEnabled = false; // 0x0000-0x1FFF, gates both RAM and RTC register access
    u8 latchTriggerLastWrite = 0xFF; // Last byte written to 0x6000-0x7FFF, to detect a 0->1 transition

    // Live (ticking) RTC registers
    u8 liveSeconds = 0;
    u8 liveMinutes = 0;
    u8 liveHours = 0;
    u16 liveDayCounter = 0; // 9-bit day count
    bool liveHalt = false;
    bool liveDayCarry = false; // Sticky, set when the day counter overflows past 511

    // Snapshot of the live registers, refreshed by a 0->1 latch-trigger write
    u8 latchedSeconds = 0;
    u8 latchedMinutes = 0;
    u8 latchedHours = 0;
    u16 latchedDayCounter = 0;
    bool latchedHalt = false;
    bool latchedDayCarry = false;

    u32 cycleAccumulator = 0; // Counts T-cycles up to CyclesPerSecond
};
