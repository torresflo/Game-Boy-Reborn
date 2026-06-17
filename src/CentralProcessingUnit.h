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
    
    const InstructionData& getInstructionFromOpCode(u8 opcode) const;

    u16 readRegister(RegisterType type) const;
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
    u8 jrInstruction();
    u8 callInstruction();
    u8 retInstruction();
    u8 retiInstruction();
    u8 rstInstruction();
    u8 popInstruction();
    u8 pushInstruction();
    u8 incInstruction();
    u8 decInstruction();
    u8 addInstruction();
    u8 adcInstruction();
    u8 subInstruction();
    u8 sbcInstruction();
    u8 diInstruction();
    u8 xorInstruction();

    u8 gotoAddress(u16 address, bool pushPC);
    bool checkCondition() const;

    bool flagZ() const;
    bool flagN() const;
    bool flagH() const;
    bool flagC() const;
    void setFlagValues(s8 zFlag, s8 nFlag, s8 hFlag, s8 cFlag);

    void stackPush(u8 data);
    void stackPush16(u16 data);
    u8 stackPop();
    u16 stackPop16();

    bool is16BitsRegister(RegisterType type) const;

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