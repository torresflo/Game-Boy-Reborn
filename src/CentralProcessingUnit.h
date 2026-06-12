#pragma once

#include "Common.h"
#include "InstructionDefinitions.h"
#include "MemoryBus.h"

struct Registers 
{
    u8 A;
    u8 F;
    u8 B;
    u8 C;
    u8 D;
    u8 E;
    u8 H;
    u8 L;
    u16 SP; //Stack Pointer
    u16 PC; //Program Counter
};

class CentralProcessingUnit
{
public:
    void initialize(MemoryBus* bus);
    u8 step(); //Return consumed cycles

private:
    void fetchInstruction();
    u8 fetchData(); //Return consumed cycles
    void execute();
    
    const Instruction& getInstructionFromOpCode(u8 opcode);

    u16 readRegister(RegisterType type);

    u16 reverse(u16 value) const;

    Registers registers;
    u16 data;
    bool destinationIsMemory;
    u16 memoryDestination;
    u8 currentOPCode;
    Instruction currentInstruction;

    bool halted = false;
    bool stepping = false;

    MemoryBus* memoryBus;
};