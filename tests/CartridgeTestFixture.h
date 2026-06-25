#pragma once

#include "Cartridge.h"

class CartridgeTestFixture
{
public:
    void setROMSize(u32 sizeBytes)
    {
        cartridge.ROMData.assign(sizeBytes, 0);
    }

    void setRAMSize(u32 sizeBytes)
    {
        cartridge.RAMData.assign(sizeBytes, 0);
    }

    void setROMByte(u32 offset, u8 value)
    {
        cartridge.ROMData[offset] = value;
    }

    void setRAMByte(u32 offset, u8 value)
    {
        cartridge.RAMData[offset] = value;
    }

    u8 getRAMByte(u32 offset) const
    {
        return cartridge.RAMData[offset];
    }

    void setHeader(u8 cartridgeType, u8 romSizeCode, u8 ramSizeCode)
    {
        cartridge.header.cartridgeType = cartridgeType;
        cartridge.header.romSize = romSizeCode;
        cartridge.header.ramSize = ramSizeCode;
    }

    u32 getRAMSize() const
    {
        return static_cast<u32>(cartridge.RAMData.size());
    }

    Cartridge& getCartridge()
    {
        return cartridge;
    }

private:
    Cartridge cartridge;
};
