#pragma once

#include <array>
#include <queue>
#include <type_traits>
#include <vector>

#include "Common.h"

class SaveStateReader
{
public:
    explicit SaveStateReader(std::vector<u8> data);

    template<typename T>
    bool read(T& outValue)
    {
        static_assert(std::is_trivially_copyable_v<T>, "SaveStateReader::read requires a trivially copyable type");
        return readBytes(reinterpret_cast<u8*>(&outValue), sizeof(T));
    }

    template<typename T, size_t N>
    bool readArray(std::array<T, N>& outValues)
    {
        static_assert(std::is_trivially_copyable_v<T>, "SaveStateReader::readArray requires a trivially copyable element type");
        return readBytes(reinterpret_cast<u8*>(outValues.data()), sizeof(T) * N);
    }

    bool readVector(std::vector<u8>& outValues);

    template<typename T>
    bool readVector(std::vector<T>& outValues)
    {
        static_assert(std::is_trivially_copyable_v<T>, "SaveStateReader::readVector requires a trivially copyable element type");
        u32 count = 0;
        if(!read(count))
            return false;

        outValues.resize(count);
        for(T& value : outValues)
        {
            if(!read(value))
                return false;
        }
        return true;
    }

    bool readQueue(std::queue<u32>& outValues);

private:
    bool readBytes(u8* destination, size_t size);

    std::vector<u8> buffer;
    size_t offset = 0;
};
