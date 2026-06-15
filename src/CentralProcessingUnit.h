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
    u8 execute();  //Return consumed cycles
    
    const InstructionData& getInstructionFromOpCode(u8 opcode);

    u16 readRegister(RegisterType type);
    void writeRegister(RegisterType type, u16 value);

    u16 reverse(u16 value) const;

    // CPU Instructions
    using InstructionFunc = u8 (CentralProcessingUnit::*)();
    InstructionFunc getInstructionFunc(InstructionType type);

    u8 noneInstruction();
    u8 nopInstruction();
    u8 ldInstruction();
    u8 ldhInstruction();
    u8 jpInstruction();
    u8 diInstruction();
    u8 xorInstruction();

    bool checkCondition() const;

    bool flagZ() const;
    bool flagC() const;
    void setFlagValues(s8 z, s8 n, s8 h, s8 c);
    
    Registers registers;
    u16 fetchedData;
    bool destinationIsMemory;
    u16 memoryDestination;
    u8 currentOPCode;
    InstructionData currentInstruction;
    
    bool interruptMasterEnabled = true;

    bool halted = false;
    bool stepping = false;
    
    MemoryBus* memoryBus;

    static const std::array<InstructionFunc, static_cast<size_t>(InstructionType::COUNT)> InstructionFuncs;
};