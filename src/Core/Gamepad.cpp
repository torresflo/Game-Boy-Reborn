#include "Gamepad.h"

#include "MathUtils.h"
#include "Save/SaveStateReader.h"
#include "Save/SaveStateWriter.h"

void Gamepad::initialize()
{
    buttonStates.fill(false);
    isSelectingButtons = false;
    isSelectingDirections = false;
}

u8 Gamepad::getAsMemoryValue() const
{
    u8 value = 0xCF;
    MathUtils<u8>::setBitValue(value, 5, !isSelectingButtons);
    MathUtils<u8>::setBitValue(value, 4, !isSelectingDirections);

    if(isSelectingDirections)
    {
        MathUtils<u8>::setBitValue(value, 3, isButtonDownForMemory(Gamepad::Button::Down));
        MathUtils<u8>::setBitValue(value, 2, isButtonDownForMemory(Gamepad::Button::Up));
        MathUtils<u8>::setBitValue(value, 1, isButtonDownForMemory(Gamepad::Button::Left));
        MathUtils<u8>::setBitValue(value, 0, isButtonDownForMemory(Gamepad::Button::Right));
    }
    else if(isSelectingButtons)
    {
        MathUtils<u8>::setBitValue(value, 3, isButtonDownForMemory(Gamepad::Button::Start));
        MathUtils<u8>::setBitValue(value, 2, isButtonDownForMemory(Gamepad::Button::Select));
        MathUtils<u8>::setBitValue(value, 1, isButtonDownForMemory(Gamepad::Button::B));
        MathUtils<u8>::setBitValue(value, 0, isButtonDownForMemory(Gamepad::Button::A));
    }

    return value;
}

void Gamepad::setFromMemory(u8 value)
{
    isSelectingButtons = !MathUtils<u8>::getBitValue(value, 5);
    isSelectingDirections = !MathUtils<u8>::getBitValue(value, 4);
}

bool Gamepad::isButtonDown(Gamepad::Button button) const
{
    u32 index = static_cast<u32>(button);
    return buttonStates[index];
}

void Gamepad::setButtonState(Gamepad::Button button, bool isDown)
{
    u32 index = static_cast<u32>(button);
    buttonStates[index] = isDown;
}

bool Gamepad::isButtonDownForMemory(Gamepad::Button button) const
{
    return !isButtonDown(button); //In memory, if bit is 0, then button is down
}

void Gamepad::serialize(SaveStateWriter& writer) const
{
    writer.write(isSelectingButtons);
    writer.write(isSelectingDirections);
}

void Gamepad::deserialize(SaveStateReader& reader)
{
    reader.read(isSelectingButtons);
    reader.read(isSelectingDirections);
}
