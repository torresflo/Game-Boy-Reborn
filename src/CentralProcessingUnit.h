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
    void step();

private:
    void fetchInstruction();
    void fetchData();
    void execute();

    const InstructionData& getInstructionFromOpCode(u8 opcode) const;

    u16 readRegister(RegisterType type) const;
    void writeRegister(RegisterType type, u16 value);

    u16 reverse(u16 value) const;

    void emulateCycles(u8 cycleCount);

    // CPU Instructions
    using InstructionFunc = void (CentralProcessingUnit::*)();
    InstructionFunc getInstructionFunc(InstructionType type);

    void noneInstruction();
    void nopInstruction();
    void andInstruction();
    void orInstruction();
    void xorInstruction();
    void cpInstruction();
    void ldInstruction();
    void ldhInstruction();
    void jpInstruction();
    void jrInstruction();
    void callInstruction();
    void retInstruction();
    void retiInstruction();
    void rstInstruction();
    void popInstruction();
    void pushInstruction();
    void incInstruction();
    void decInstruction();
    void addInstruction();
    void adcInstruction();
    void subInstruction();
    void sbcInstruction();
    void diInstruction();
    void cbInstruction();

    u8 readRegisterForCBInstruction(RegisterType type) const;
    void writeRegisterForCBInstruction(RegisterType type, u8 value);

    void gotoAddress(u16 address, bool pushPC);
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

    u64 cycles = 0;

    MemoryBus* memoryBus;

    static const std::array<InstructionFunc, static_cast<size_t>(InstructionType::COUNT)> InstructionFuncs;

    friend class CentralProcessingUnitTestFixture;
};