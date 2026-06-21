#pragma once
#include <array>

#include "Common.h"
#include "Timer.h"
#include "PixelProcessingUnitTypes.h"

class Cartridge;
class CentralProcessingUnit;

struct DirectMemoryAccessContext
{
    bool isActive = false;
    u8 index;
    u8 value;
    u8 startDelay;
};

class MemoryBus
{
public:
    virtual ~MemoryBus() = default;

    void initialize(Cartridge* cartridgePtr, CentralProcessingUnit* cpuPtr);

    virtual u8 read(u16 address) const;
    u16 read16(u16 address) const;

    virtual void write(u16 address, u8 value);
    void write16(u16 address, u16 value);

    //OAM Access
    ObjectAttributeMemoryEntry readObject(u16 address) const;
    void writeObject(u16 address, ObjectAttributeMemoryEntry object);

    //DMA
    void startDMA(u8 startValue);
    void tickDMATransfer();
    bool isDMATransferInProgress() const;

    //Timer
    void tickTimer();

    // Used by the CPU's interrupt dispatch logic
    virtual u8 readInterruptEnableRegister() const;
    virtual void writeInterruptEnableRegister(u8 value);
    virtual u8 readInterruptFlags() const;
    virtual void writeInterruptFlags(u8 value);

private:
    u8 readWRAM(u16 address) const;
    void writeWRAM(u16 address, u8 value);

    u8 readHRAM(u16 address) const;
    void writeHRAM(u16 address, u8 value);

    u8 readOAM(u16 address) const;
    void writeOAM(u16 address, u8 value);

    u8 readVRAM(u16 address) const;
    void writeVRAM(u16 address, u8 value);

    u8 readIO(u16 address) const;
    void writeIO(u16 address, u8 value);

    //RAM
    std::array<u8, 0x2000> WRAM;
    std::array<u8, 0x80> HRAM;

    //OAM and VRAM
    std::array<ObjectAttributeMemoryEntry, 40> OAM;
    std::array<u8, 0x2000> VRAM;

    //OAM DMA Transfer
    DirectMemoryAccessContext DMAContext;
    
    // IO
    std::array<s8, 2> serialData;
    HardwareTimer timer;
    u8 interruptFlags = 0; //IF

    //IE
    u8 interruptEnableRegister;

    Cartridge* cartridge = nullptr;
};