#pragma once

#include "Common.h"
#include "CentralProcessingUnitTypes.h"
#include "InstructionDefinitions.h"
#include "MemoryBus.h"

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
    void eiInstruction();
    void diInstruction();
    void cbInstruction();
    void rlcaInstruction();
    void rrcaInstruction();
    void rlaInstruction();
    void rraInstruction();
    void daaInstruction();
    void cplInstruction();
    void scfInstruction();
    void ccfInstruction();
    void haltInstruction();
    void stopInstruction();

    u8 readRegisterForCBInstruction(RegisterType type) const;
    void writeRegisterForCBInstruction(RegisterType type, u8 value);

    void gotoAddress(u16 address, bool pushPC);
    bool checkCondition() const;

    bool flagZ() const;
    bool flagN() const;
    bool flagH() const;
    bool flagC() const;
    void setFlagValues(s8 zFlag, s8 nFlag, s8 hFlag, s8 cFlag);

    u8 getInterruptFlags() const;
    void setInterruptFlags(u8 value);
    void handleInterrupts();
    bool handleInterrupt(InterruptType type, u16 address);
    void requestInterrupt(InterruptType type);

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
    
    bool interruptMasterEnabled = false;
    bool enablingInterruptMaster = false;

    u8 interruptEnable = 0; //IE
    u8 interruptFlags = 0; //IF

    bool halted = false;
    bool stepping = false;

    u64 cycles = 0;

    MemoryBus* memoryBus;

    static const std::array<InstructionFunc, static_cast<size_t>(InstructionType::COUNT)> InstructionFuncs;

    friend class CentralProcessingUnitJsonTestFixture;
};