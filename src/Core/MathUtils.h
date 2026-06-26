#pragma once

#include "Common.h"

template <typename T>
class MathUtils
{
public:
    static bool getBitValue(T value, u32 bitPosition);
    static void setBitValue(T& value, u32 bitPosition, bool bitValue);
    static bool isBetween(T value, T minIncluded, T maxIncluded);
};

template <typename T>
inline bool MathUtils<T>::getBitValue(T value, u32 bitPosition)
{
    return static_cast<bool>(value & (1 << bitPosition));
}

template <typename T>
inline void MathUtils<T>::setBitValue(T &value, u32 bitPosition, bool bitValue)
{
    if(bitValue)
        value |= (1 << bitPosition);
    else
        value &= ~(1 << bitPosition);
}

template <typename T>
inline bool MathUtils<T>::isBetween(T value, T minIncluded, T maxIncluded)
{
    return value >= minIncluded && value <= maxIncluded;
}
