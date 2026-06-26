#include "CentralProcessingUnit.h"

#include "MemoryBus.h"

void CentralProcessingUnit::fetchData()
{
    memoryDestination = 0;
    destinationIsMemory = false;

    switch(currentInstruction.addressMode)
    {
        case AddressMode::IMPLY:
            break;
        case AddressMode::R:
        {
            fetchedData = readRegister(currentInstruction.register1);
            break;
        }
        case AddressMode::R_R:
        {
            fetchedData = readRegister(currentInstruction.register2);
            break;
        }
        case AddressMode::D8:
        case AddressMode::R_D8:
        {
            fetchedData = memoryBus->read(registers.PC);
            emulateCycles(1);
            registers.PC++;
            break;
        }
        case AddressMode::R_D16:
        case AddressMode::D16:
        {
            u16 lo = memoryBus->read(registers.PC);
            emulateCycles(1);
            u16 hi = memoryBus->read(registers.PC + 1);
            emulateCycles(1);
            fetchedData = (hi << 8) | lo;
            registers.PC += 2;
            break;
        }
        case AddressMode::MR_R:
        {
            fetchedData = readRegister(currentInstruction.register2);
            memoryDestination = readRegister(currentInstruction.register1);
            destinationIsMemory = true;
            if(currentInstruction.register1 == RegisterType::C)
            {
                memoryDestination |= 0xFF00;
            }
            break;
        }
        case AddressMode::R_MR:
        {
            u16 address = readRegister(currentInstruction.register2);
            if(currentInstruction.register2 == RegisterType::C)
            {
                address |= 0xFF00;
            }
            fetchedData = memoryBus->read(address);
            emulateCycles(1);
            break;
        }
        case AddressMode::R_HLI:
        {
            fetchedData = memoryBus->read(readRegister(currentInstruction.register2));
            emulateCycles(1);
            registers.setHL(registers.getHL() + 1);
            break;
        }
        case AddressMode::R_HLD:
        {
            fetchedData = memoryBus->read(readRegister(currentInstruction.register2));
            emulateCycles(1);
            registers.setHL(registers.getHL() - 1);
            break;
        }
        case AddressMode::HLI_R:
        {
            fetchedData = readRegister(currentInstruction.register2);
            memoryDestination = readRegister(currentInstruction.register1);
            destinationIsMemory = true;
            registers.setHL(registers.getHL() + 1);
            break;
        }
        case AddressMode::HLD_R:
        {
            fetchedData = readRegister(currentInstruction.register2);
            memoryDestination = readRegister(currentInstruction.register1);
            destinationIsMemory = true;
            registers.setHL(registers.getHL() - 1);
            break;
        }
        case AddressMode::R_A8:
        {
            fetchedData = memoryBus->read(registers.PC);
            emulateCycles(1);
            registers.PC++;
            break;
        }
        case AddressMode::A8_R:
        {
            memoryDestination = memoryBus->read(registers.PC) | 0xFF00;
            destinationIsMemory = true;
            emulateCycles(1);
            registers.PC++;
            break;
        }
        case AddressMode::A16_R:
        case AddressMode::D16_R:
        {
            u16 lo = memoryBus->read(registers.PC);
            emulateCycles(1);
            u16 hi = memoryBus->read(registers.PC + 1);
            emulateCycles(1);
            memoryDestination = (hi << 8) | lo;
            destinationIsMemory = true;
            registers.PC += 2;
            fetchedData = readRegister(currentInstruction.register2);
            break;
        }
        case AddressMode::MR_D8:
        {
            fetchedData = memoryBus->read(registers.PC);
            emulateCycles(1);
            registers.PC++;
            memoryDestination = readRegister(currentInstruction.register1);
            destinationIsMemory = true;
            break;
        }
        case AddressMode::MR:
        {
            memoryDestination = readRegister(currentInstruction.register1);
            destinationIsMemory = true;
            fetchedData = memoryBus->read(memoryDestination);
            emulateCycles(1);
            break;
        }
        case AddressMode::HL_SPR:
        {
            fetchedData = memoryBus->read(registers.PC);
            emulateCycles(1);
            registers.PC++;
            break;
        }
        case AddressMode::R_A16:
        {
            u16 lo = memoryBus->read(registers.PC);
            emulateCycles(1);
            u16 hi = memoryBus->read(registers.PC + 1);
            emulateCycles(1);
            u16 address = (hi << 8) | lo;

            registers.PC += 2;
            fetchedData = memoryBus->read(address);
            emulateCycles(1);
            break;
        }
        default:
            Log::print(LogLevel::Error, "Unimplemented address mode.");
            break;
    }
}
