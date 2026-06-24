#include "MBC3.h"

#include "MathUtils.h"

MBC3::MBC3(Cartridge* cartridgePtr, u32 romBankCount, u32 ramBankCount, bool hasBattery, bool hasTimer)
    : MemoryBankController(cartridgePtr, hasBattery)
    , ROMBankCount(romBankCount)
    , RAMBankCount(ramBankCount)
    , timerPresent(hasTimer)
{
}

u8 MBC3::read(u16 address) const
{
    if(address < 0x4000)
        return getROMData()[address];

    if(address < 0x8000)
    {
        u32 bank = romBankNumber & (ROMBankCount - 1);
        return getROMData()[bank * ROMBankSizeBytes + (address - 0x4000)];
    }

    //Cartridge RAM / RTC registers, 0xA000-0xBFFF
    if(!ramAndTimerEnabled)
        return 0xFF;

    if(ramOrRTCSelect <= 0x03)
    {
        if(RAMBankCount == 0)
            return 0xFF;

        u32 bank = ramOrRTCSelect & (RAMBankCount - 1);
        return getRAMData()[bank * RAMBankSizeBytes + (address - 0xA000)];
    }

    if(isRTCRegisterSelected())
    {
        switch(ramOrRTCSelect)
        {
            case 0x08:
                return latchedSeconds;
            case 0x09:
                return latchedMinutes;
            case 0x0A:
                return latchedHours;
            case 0x0B:
                return static_cast<u8>(latchedDayCounter & 0xFF);
            case 0x0C:
            {
                u8 dayHigh = 0;
                MathUtils<u8>::setBitValue(dayHigh, 0, MathUtils<u16>::getBitValue(latchedDayCounter, 8));
                MathUtils<u8>::setBitValue(dayHigh, 6, latchedHalt);
                MathUtils<u8>::setBitValue(dayHigh, 7, latchedDayCarry);
                return dayHigh;
            }
        }
    }

    return 0xFF;
}

void MBC3::write(u16 address, u8 value)
{
    if(address < 0x2000)
    {
        ramAndTimerEnabled = (value & 0x0F) == 0x0A;
    }
    else if(address < 0x4000)
    {
        u8 bankBits = static_cast<u8>(value & 0x7F);
        romBankNumber = (bankBits == 0) ? 1 : bankBits; //Bank 0 is not selectable, the chip substitutes bank 1
    }
    else if(address < 0x6000)
    {
        ramOrRTCSelect = value;
    }
    else if(address < 0x8000)
    {
        if(latchTriggerLastWrite == 0x00 && value == 0x01)
            latchRTC();

        latchTriggerLastWrite = value;
    }
    else if(ramAndTimerEnabled)
    {
        if(ramOrRTCSelect <= 0x03)
        {
            if(RAMBankCount == 0)
                return;

            u32 bank = ramOrRTCSelect & (RAMBankCount - 1);
            getRAMData()[bank * RAMBankSizeBytes + (address - 0xA000)] = value;
        }
        else if(isRTCRegisterSelected())
        {
            switch(ramOrRTCSelect)
            {
                case 0x08:
                    liveSeconds = value;
                    break;
                case 0x09:
                    liveMinutes = value;
                    break;
                case 0x0A:
                    liveHours = value;
                    break;
                case 0x0B:
                    liveDayCounter = (liveDayCounter & 0x100) | value;
                    break;
                case 0x0C:
                    liveDayCounter = (liveDayCounter & 0xFF) | (MathUtils<u8>::getBitValue(value, 0) ? 0x100 : 0);
                    liveHalt = MathUtils<u8>::getBitValue(value, 6);
                    liveDayCarry = MathUtils<u8>::getBitValue(value, 7); //Writable, so a sticky carry can be cleared manually
                    break;
            }
        }
    }
}

void MBC3::tick()
{
    if(!timerPresent || liveHalt)
        return;

    cycleAccumulator++;
    if(cycleAccumulator >= CyclesPerSecond)
    {
        cycleAccumulator -= CyclesPerSecond;
        advanceRTCBySecond();
    }
}

bool MBC3::isRTCRegisterSelected() const
{
    return timerPresent && MathUtils<u8>::isBetween(ramOrRTCSelect, RTCRegisterSelectMin, RTCRegisterSelectMax);
}

void MBC3::advanceRTCBySecond()
{
    liveSeconds++;
    if(liveSeconds < 60)
        return;
    liveSeconds = 0;

    liveMinutes++;
    if(liveMinutes < 60)
        return;
    liveMinutes = 0;

    liveHours++;
    if(liveHours < 24)
        return;
    liveHours = 0;

    liveDayCounter++;
    if(liveDayCounter > 0x1FF) //Overflowed past day 511, the 9-bit counter's max
    {
        liveDayCounter = 0;
        liveDayCarry = true;
    }
}

void MBC3::latchRTC()
{
    latchedSeconds = liveSeconds;
    latchedMinutes = liveMinutes;
    latchedHours = liveHours;
    latchedDayCounter = liveDayCounter;
    latchedHalt = liveHalt;
    latchedDayCarry = liveDayCarry;
}
