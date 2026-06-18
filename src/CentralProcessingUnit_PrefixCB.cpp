#include "CentralProcessingUnit.h"
#include "MathUtils.h"

#include <format>
#include <array>

static const std::array<RegisterType, 8> CBRegisterLookupTable =
{
    RegisterType::B,
    RegisterType::C,
    RegisterType::D,
    RegisterType::E,
    RegisterType::H,
    RegisterType::L,
    RegisterType::HL,
    RegisterType::A,
};

RegisterType decodeRegisterForCBInstruction(u8 value)
{
    if(value > 0b111)
        return RegisterType::NONE;
    
    return CBRegisterLookupTable[value]; 
}

u8 CentralProcessingUnit::readRegisterForCBInstruction(RegisterType type) const
{
    switch (type)
    {
        case RegisterType::A:
            return registers.A;
        case RegisterType::F:
            return registers.F;
        case RegisterType::B:
            return registers.B;
        case RegisterType::C:
            return registers.C;
        case RegisterType::D:
            return registers.D;
        case RegisterType::E:
            return registers.E;
        case RegisterType::H:
            return registers.H;
        case RegisterType::L:
            return registers.L;
        case RegisterType::HL:
            return memoryBus->read(registers.getHL());
        default:
            Log::print(LogLevel::Error, "Invalid register type (read)");
            return 0;
    }
}

void CentralProcessingUnit::writeRegisterForCBInstruction(RegisterType type, u8 value)
{
    switch (type)
    {
        case RegisterType::A:
            registers.A = value & 0xFF;
            break;
        case RegisterType::F:
            registers.F = value & 0xFF;
            break;
        case RegisterType::B:
            registers.B = value & 0xFF;
            break;
        case RegisterType::C:
            registers.C = value & 0xFF;
            break;
        case RegisterType::D:
            registers.D = value & 0xFF;
            break;
        case RegisterType::E:
            registers.E = value & 0xFF;
            break;
        case RegisterType::H:
            registers.H = value & 0xFF;
            break;
        case RegisterType::L:
            registers.L = value & 0xFF;
            break;
        case RegisterType::HL:
            memoryBus->write(registers.getHL(), value);
            break;
        default:
            Log::print(LogLevel::Error, "Invalid register type (write)");
            break;
    }
}

void CentralProcessingUnit::cbInstruction()
{
    u8 operation = static_cast<u8>(fetchedData);
    RegisterType registerType = decodeRegisterForCBInstruction(operation & 0b111);
    u8 bitPosition = (operation >> 3) & 0b111;
    u8 bitOperation = (operation >> 6) & 0b11;
    u8 registerValue = readRegisterForCBInstruction(registerType);

    emulateCycles(1);

    if(registerType == RegisterType::HL)
        emulateCycles(2);

    switch(bitOperation)
    {
        case 1: //BIT
            setFlagValues(!MathUtils<u8>::getBitValue(registerValue, bitPosition), 0, 1, -1);
            return;

        case 2: //RST
            MathUtils<u8>::setBitValue(registerValue, bitPosition, false);
            writeRegisterForCBInstruction(registerType, registerValue);
            return;

        case 3: //SET
            MathUtils<u8>::setBitValue(registerValue, bitPosition, true);
            writeRegisterForCBInstruction(registerType, registerValue);
            return;
    }

    u8 flagCAsU8 = static_cast<u8>(flagC());
    switch(bitPosition)
    {
        case 0: //RLC
        {
            bool setCFlag = MathUtils<u8>::getBitValue(registerValue, 7);
            u8 result = (registerValue << 1) & 0xFF;
            MathUtils<u8>::setBitValue(result, 0, setCFlag);

            writeRegisterForCBInstruction(registerType, result);
            setFlagValues(result == 0, 0, 0, setCFlag);
            return;
        }
        
        case 1: //RRC
        {
            u8 oldValue = registerValue;
            registerValue >>= 1;
            MathUtils<u8>::setBitValue(registerValue, 7, MathUtils<u8>::getBitValue(oldValue, 0));

            writeRegisterForCBInstruction(registerType, registerValue);
            setFlagValues(registerValue == 0, 0, 0, MathUtils<u8>::getBitValue(oldValue, 0));
            return;
        }

        case 2: //RL
        {
            u8 oldValue = registerValue;
            registerValue <<= 1;
            MathUtils<u8>::setBitValue(registerValue, 0, flagCAsU8);

            writeRegisterForCBInstruction(registerType, registerValue);
            setFlagValues(registerValue == 0, 0, 0, MathUtils<u8>::getBitValue(oldValue, 7));
            return;
        }

        case 3: //RR
        {
            u8 oldValue = registerValue;
            registerValue >>= 1;
            MathUtils<u8>::setBitValue(registerValue, 7, flagCAsU8);

            writeRegisterForCBInstruction(registerType, registerValue);
            setFlagValues(registerValue == 0, 0, 0, MathUtils<u8>::getBitValue(oldValue, 0));
            return;
        }

        case 4: //SLA
        {
            u8 oldValue = registerValue;
            registerValue <<= 1;

            writeRegisterForCBInstruction(registerType, registerValue);
            setFlagValues(registerValue == 0, 0, 0, MathUtils<u8>::getBitValue(oldValue, 7));
            return;
        }

        case 5: //SRA
        {
            u8 value = static_cast<s8>(registerValue) >> 1;
            writeRegisterForCBInstruction(registerType, value);
            setFlagValues(value == 0, 0, 0, MathUtils<u8>::getBitValue(registerValue, 0));
            return;
        }

        case 6: //SWAP
        {
            registerValue = ((registerValue & 0xF0) >> 4) | ((registerValue & 0xF) << 4);
            writeRegisterForCBInstruction(registerType, registerValue);
            setFlagValues(registerValue == 0, 0, 0, 0);
            return;
        }

        case 7: //SRL
        {
            u8 value = registerValue >> 1;
            writeRegisterForCBInstruction(registerType, value);
            setFlagValues(value == 0, 0, 0, MathUtils<u8>::getBitValue(registerValue, 0));
            return;
        }
    }

    Log::print(LogLevel::Error, std::format("Invalid CB instruction {:02X}", operation));
}