#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>

#include "Common.h"
#include "Save/ISaveStateSerializable.h"
#include "MBC/MemoryBankController.h"

class MemoryBankControllerFactory;

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

class Cartridge : public ISaveStateSerializable
{
public:
    ~Cartridge();

    bool loadROM(std::string path);

    const std::string& getRomPath() const;
    const CartridgeHeader& getHeader() const;
    std::string getRomTypeName(u8 type) const;
    std::string getLicenceName(u8 code) const;
    bool checkHeaderChecksum() const;

    bool saveRAM() const;

    virtual void serialize(SaveStateWriter& writer) const override;
    virtual void deserialize(SaveStateReader& reader) override;

private:
    u8 read(u16 address) const;
    void write(u16 address, u8 value);
    void tick();

    std::string getSaveFilePath() const;
    void loadRAMFromDisk();

    std::string ROMPath;
    u32 ROMSize;
    std::vector<u8> ROMData;
    std::vector<u8> RAMData;
    std::unique_ptr<MemoryBankController> mbc;
    CartridgeHeader header;

    static const std::vector<std::string> RomTypes;
    static const std::map<u8, std::string> LicenceCodes;

    friend class MemoryBus;
    friend class MemoryBankController;
    friend class MemoryBankControllerFactory;
    friend class CartridgeTestFixture;
};
