#include "MemoryBankController.h"

#include "Cartridge.h"

MemoryBankController::MemoryBankController(Cartridge* cartridgePtr, bool hasBatteryFlag)
    : cartridge(cartridgePtr)
    , batteryPresent(hasBatteryFlag)
{
}

void MemoryBankController::tick()
{
}

void MemoryBankController::serialize(SaveStateWriter&) const
{
}

void MemoryBankController::deserialize(SaveStateReader&)
{
}

bool MemoryBankController::hasBattery() const
{
    return batteryPresent;
}

const std::vector<u8>& MemoryBankController::getROMData() const
{
    return cartridge->ROMData;
}

std::vector<u8>& MemoryBankController::getRAMData()
{
    return cartridge->RAMData;
}

const std::vector<u8>& MemoryBankController::getRAMData() const
{
    return cartridge->RAMData;
}
