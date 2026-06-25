#include "MemoryBus.h"

#include <format>

#include "AudioProcessingUnit.h"
#include "Cartridge.h"
#include "PixelProcessingUnit.h"
#include "Gamepad.h"

// 0x0000 - 0x3FFF : ROM Bank 0
// 0x4000 - 0x7FFF : ROM Bank 1 - Switchable
// 0x8000 - 0x97FF : CHR RAM
// 0x9800 - 0x9BFF : BG Map 1 (VRAM)
// 0x9C00 - 0x9FFF : BG Map 2 (VRAM)
// 0xA000 - 0xBFFF : Cartridge RAM
// 0xC000 - 0xCFFF : RAM Bank 0
// 0xD000 - 0xDFFF : RAM Bank 1-7 - switchable - Color only
// 0xE000 - 0xFDFF : Reserved - Echo RAM
// 0xFE00 - 0xFE9F : OAM (Object Attribute Memory)
// 0xFEA0 - 0xFEFF : Reserved - Unusable
// 0xFF00 - 0xFF7F : I/O Registers
// 0xFF80 - 0xFFFE : Zero Page / HRAM

void MemoryBus::initialize(Cartridge* cartridgePtr, CentralProcessingUnit* cpuPtr, PixelProcessingUnit* ppuPtr, AudioProcessingUnit* apuPtr, Gamepad* gamepadPtr)
{
    cartridge = cartridgePtr;
    ppu = ppuPtr;
    apu = apuPtr;
    timer.initialize(cpuPtr);
    gamepad = gamepadPtr;

    WRAM.fill(0);
    HRAM.fill(0);
    OAM.fill(ObjectAttributeMemoryEntry{});
    VRAM.fill(0);
    serialData.fill(0);

    dmaRegister = 0;

    interruptEnableRegister = 0;
    interruptFlags = 0;
}

u8 MemoryBus::read(u16 address) const
{
    if(address < 0x8000)
    {
        //ROM Banks
        if(cartridge != nullptr)
            return cartridge->read(address);

        Log::print(LogLevel::Error, std::format("Reading ROM (0x{:4X}) with no cartridge attached.", address));
        return 0xFF;
    }
    else if(address < 0xA000)
    {
        //VRAM
        return readVRAM(address);
    }
    else if(address < 0xC000)
    {
        //Cartridge RAM
        if(cartridge != nullptr)
            return cartridge->read(address);

        Log::print(LogLevel::Error, std::format("Reading cartridge RAM (0x{:4X}) with no cartridge attached.", address));
        return 0xFF;
    }
    else if(address < 0xE000)
    {
        //RAM Banks - WRAM (working RAM)
        return readWRAM(address);
    }
    else if(address < 0xFE00)
    {
        //Reserved Echo RAM
        Log::print(LogLevel::Error, std::format("Unsupported bus reading (0x{:4X}).", address));
    }
    else if(address < 0xFEA0)
    {
        //OAM
        if(DMAContext.isActive)
            return 0xFF;

        return readOAM(address);
    }
    else if(address < 0xFF00)
    {
        //Reserved, unusable
        Log::print(LogLevel::Error, std::format("Unsupported bus reading (0x{:4X}).", address));
    }
    else if(address < 0xFF80)
    {
        //IO Registers
        return readIO(address);
    }
    else if(address == 0xFFFF)
    {
        return readInterruptEnableRegister();
    }
    else
    {
        return readHRAM(address);
    }

    return 0;
}

u16 MemoryBus::read16(u16 address) const
{
    //Implicit conversion to 16 bits
    u16 lo = read(address);
    u16 hi = read(address +1);

    return (hi << 8) | lo;
}

void MemoryBus::write(u16 address, u8 value)
{
    if(address < 0x8000)
    {
        if(cartridge != nullptr)
            cartridge->write(address, value);
        else
            Log::print(LogLevel::Error, std::format("Writing ROM (0x{:4X}) with no cartridge attached.", address));
    }
    else if(address < 0xA000)
    {
        //VRAM
        writeVRAM(address, value);
    }
    else if(address < 0xC000)
    {
        //Cartridge RAM
        if(cartridge != nullptr)
            cartridge->write(address, value);
        else
            Log::print(LogLevel::Error, std::format("Writing cartridge RAM (0x{:4X}) with no cartridge attached.", address));
    }
    else if(address < 0xE000)
    {
        //RAM Banks - WRAM (working RAM)
        writeWRAM(address, value);
    }
    else if(address < 0xFE00)
    {
        //Reserved Echo RAM
        Log::print(LogLevel::Error, std::format("Unsupported bus writing (0x{:4X}).", address));
    }
    else if(address < 0xFEA0)
    {
        //OAM
        if(DMAContext.isActive)
            return;

        writeOAM(address, value);
    }
    else if(address < 0xFF00)
    {
        //Reserved, unusable
        Log::print(LogLevel::Error, std::format("Unsupported bus writing (0x{:4X}).", address));
    }
    else if(address < 0xFF80)
    {
        //IO Registers
        writeIO(address, value);
    }
    else if(address == 0xFFFF)
    {
        //CPU Enable Register
        writeInterruptEnableRegister(value);
    }
    else
    {
        writeHRAM(address, value);
    }
}

void MemoryBus::write16(u16 address, u16 value)
{
    write(address, value & 0xFF);
    write(address + 1, (value >> 8) & 0xFF);
}

ObjectAttributeMemoryEntry MemoryBus::readObject(u16 address) const
{
    if(address >= 0xFE00)
        address -= 0xFE00;

    return OAM[address];
}

void MemoryBus::writeObject(u16 address, ObjectAttributeMemoryEntry object)
{
    if(address >= 0xFE00)
        address -= 0xFE00;
    
    OAM[address] = object;
}

void MemoryBus::startDMA(u8 startValue)
{
    DMAContext.isActive = true;
    DMAContext.index = 0;
    DMAContext.startDelay = 2;
    DMAContext.value = startValue;
}

void MemoryBus::tickDMATransfer()
{
    if(!isDMATransferInProgress())
        return;

    if(DMAContext.startDelay > 0)
    {
        DMAContext.startDelay--;
        return;
    }

    writeOAM(DMAContext.index, read(DMAContext.value * 0x100 + DMAContext.index));

    DMAContext.index++;
    DMAContext.isActive = DMAContext.index < 0xA0;
}

bool MemoryBus::isDMATransferInProgress() const
{
    return DMAContext.isActive;
}

void MemoryBus::tickTimer()
{
    timer.tick();
}

void MemoryBus::tickCartridge()
{
    if(cartridge != nullptr)
        cartridge->tick();
}

u8 MemoryBus::readWRAM(u16 address) const
{
    u16 offset = address - 0xC000;
    if(offset >= WRAM.size())
    {
        Log::print(LogLevel::Fatal, std::format("WRAM read out of range (0x{:4X}).", address));
        exit(-1);
    }

    return WRAM[offset];
}

void MemoryBus::writeWRAM(u16 address, u8 value)
{
    u16 offset = address - 0xC000;
    if(offset >= WRAM.size())
    {
        Log::print(LogLevel::Fatal, std::format("WRAM write out of range (0x{:4X}).", address));
        exit(-1);
    }

    WRAM[offset] = value;
}

u8 MemoryBus::readHRAM(u16 address) const
{
    u16 offset = address - 0xFF80;
    if(offset >= HRAM.size())
    {
        Log::print(LogLevel::Fatal, std::format("HRAM read out of range (0x{:4X}).", address));
        exit(-1);
    }

    return HRAM[offset];
}

void MemoryBus::writeHRAM(u16 address, u8 value)
{
    u16 offset = address - 0xFF80;
    if(offset >= HRAM.size())
    {
        Log::print(LogLevel::Fatal, std::format("HRAM write out of range (0x{:4X}).", address));
        exit(-1);
    }

    HRAM[offset] = value;
}

u8 MemoryBus::readOAM(u16 address) const
{
    if(address >= 0xFE00)
        address -= 0xFE00;

    const u8* ptr = reinterpret_cast<const u8*>(OAM.data());
    return ptr[address];
}

void MemoryBus::writeOAM(u16 address, u8 value)
{
    if(address >= 0xFE00)
        address -= 0xFE00;

    u8* ptr = reinterpret_cast<u8*>(OAM.data());
    ptr[address] = value;
}

u8 MemoryBus::readVRAM(u16 address) const
{
    if(address >= 0x8000)
        address -= 0x8000;

    return VRAM[address];
}

void MemoryBus::writeVRAM(u16 address, u8 value)
{
    if(address >= 0x8000)
        address -= 0x8000;

    VRAM[address] = value;
}

u8 MemoryBus::readInterruptEnableRegister() const
{
    return interruptEnableRegister;
}

void MemoryBus::writeInterruptEnableRegister(u8 value)
{
    interruptEnableRegister = value;
}

u8 MemoryBus::readInterruptFlags() const
{
    return interruptFlags;
}

void MemoryBus::writeInterruptFlags(u8 value)
{
    interruptFlags = value;
}

u8 MemoryBus::readIO(u16 address) const
{
    if(address == 0xFF00)
        return gamepad->getAsMemoryValue();

    if(address == 0xFF01)
        return serialData[0];

    if(address == 0xFF02)
        return serialData[1];

    if(address >= 0xFF04 && address <= 0xFF07)
        return timer.readTimer(address);

    if(address == 0xFF0F)
        return readInterruptFlags();

    if(address == 0xFF46)
        return dmaRegister;

    if(address >= 0xFF10 && address <= 0xFF26)
    {
        if(apu != nullptr)
            return apu->readRegister(address);

        Log::print(LogLevel::Error, std::format("Reading sound register (0x{:4X}) with no APU attached.", address));
        return 0xFF;
    }

    if(address >= 0xFF30 && address <= 0xFF3F)
    {
        if(apu != nullptr)
            return apu->readWaveRAM(address);

        Log::print(LogLevel::Error, std::format("Reading wave RAM (0x{:4X}) with no APU attached.", address));
        return 0xFF;
    }

    if(address >= 0xFF40 && address <= 0xFF4B)
    {
        if(ppu != nullptr)
            return ppu->readRegister(address);

        Log::print(LogLevel::Error, std::format("Reading LCD register (0x{:4X}) with no PPU attached.", address));
        return 0xFF;
    }

    Log::print(LogLevel::Error, std::format("Unsupported IO reading (0x{:4X}).", address));
    return 0;
}

void MemoryBus::writeIO(u16 address, u8 value)
{
    if(address == 0xFF00)
        gamepad->setFromMemory(value);
    else if(address == 0xFF01)
        serialData[0] = value;
    else if(address == 0xFF02)
        serialData[1] = value;
    else if(address >= 0xFF04 && address <= 0xFF07)
        timer.writeTimer(address, value);
    else if(address == 0xFF0F)
        writeInterruptFlags(value);
    else if(address == 0xFF46)
    {
        dmaRegister = value;
        startDMA(value);
    }
    else if(address >= 0xFF10 && address <= 0xFF26)
    {
        if(apu != nullptr)
            apu->writeRegister(address, value);
        else
            Log::print(LogLevel::Error, std::format("Writing sound register (0x{:4X}) with no APU attached.", address));
    }
    else if(address >= 0xFF30 && address <= 0xFF3F)
    {
        if(apu != nullptr)
            apu->writeWaveRAM(address, value);
        else
            Log::print(LogLevel::Error, std::format("Writing wave RAM (0x{:4X}) with no APU attached.", address));
    }
    else if(address >= 0xFF40 && address <= 0xFF4B)
    {
        if(ppu != nullptr)
            ppu->writeRegister(address, value);
        else
            Log::print(LogLevel::Error, std::format("Writing LCD register (0x{:4X}) with no PPU attached.", address));
    }
    else
        Log::print(LogLevel::Error, std::format("Unsupported IO writing (0x{:4X}).", address));
}
