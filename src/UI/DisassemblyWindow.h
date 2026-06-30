#pragma once

#include <string>

#include <imgui.h>

#include "ToolWindow.h"
#include "Common.h"

class GameBoyEmulator;
class MemoryBus;
struct InstructionData;

class DisassemblyWindow : public ToolWindow
{
public:
    DisassemblyWindow();

protected:
    void drawContent(GameBoyEmulator& emulator) override;

private:
    struct DecodedInstruction
    {
        std::string bytes;
        std::string mnemonic;
        u8 length;
    };

    DecodedInstruction decodeInstruction(const MemoryBus& bus, u16 address) const;
    std::string formatOperands(const InstructionData& data, const MemoryBus& bus, u16 address) const;
    std::string formatCBInstruction(u8 cbByte) const;

    bool followProgramCounter = true;
    u16 viewAddress = 0x0000;

    static constexpr unsigned int WindowWidth = 550;
    static constexpr unsigned int WindowHeight = 400;
    static constexpr int InstructionCount = 50;

    static constexpr ImVec4 AddressColor{0.60f, 0.60f, 0.65f, 1.f};
    static constexpr ImVec4 BytesColor{0.55f, 0.55f, 0.55f, 1.f};
    static constexpr ImVec4 MnemonicColor{1.0f, 0.85f, 0.35f, 1.f};
    static constexpr ImVec4 PCHighlightColor{0.95f, 0.50f, 0.10f, 0.35f};
};
