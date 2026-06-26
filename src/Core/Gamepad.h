#pragma once

#include <array>

#include "Common.h"
#include "Save/ISaveStateSerializable.h"

class MemoryBus;

class Gamepad : public ISaveStateSerializable
{
public:
    static constexpr u8 ButtonCount = 8;
    enum class Button
    {
        Start,
        Select,
        A,
        B,
        Up,
        Down,
        Left,
        Right,
    };

    void initialize();

    u8 getAsMemoryValue() const;
    void setFromMemory(u8 value);

    bool isButtonDown(Gamepad::Button button) const;
    void setButtonState(Gamepad::Button button, bool isDown);

    virtual void serialize(SaveStateWriter& writer) const override;
    virtual void deserialize(SaveStateReader& reader) override;

private:
    bool isButtonDownForMemory(Gamepad::Button button) const;

    bool isSelectingButtons = false;
    bool isSelectingDirections = false;
    std::array<bool, ButtonCount> buttonStates;
};