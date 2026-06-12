#pragma once

#include <string>
#include <map>
#include <vector>

#include "Common.h"

struct CartridgeHeader
{
    u8 entry[4];
    u8 logo[0x30];

    char title[16];
    u16 newLicenceCode;
    u8 sgbFlag;
    u8 cartridgeType;
    u8 romSize;
    u8 ramSize;
    u8 destinationCode;
    u8 oldLicenceCode;
    u8 maskRomVersion;
    u8 headerChecksum;
    u16 globalChecksum;
};

class Cartridge
{
public:
    bool loadROM(std::string path);

private:
    std::string getRomTypeName(u8 type);
    std::string getLicenceName(u8 code);
    bool checkHeaderChecksum();

    u8 read(u16 address);
    void write(u16 address, u8 value);

    std::string ROMPath;
    u32 ROMSize;
    std::vector<u8> ROMData;
    CartridgeHeader header;

    static const std::vector<std::string> RomTypes;
    static const std::map<u8, std::string> LicenceCodes;

    friend class MemoryBus;
};
