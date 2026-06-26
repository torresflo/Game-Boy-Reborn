#pragma once

#include <array>
#include <queue>
#include <type_traits>
#include <vector>

#include "Common.h"

class SaveStateWriter
{
public:
    template<typename T>
    void write(const T& value)
    {
        static_assert(std::is_trivially_copyable_v<T>, "SaveStateWriter::write requires a trivially copyable type");
        appendBytes(reinterpret_cast<const u8*>(&value), sizeof(T));
    }

    template<typename T, size_t N>
    void writeArray(const std::array<T, N>& values)
    {
        static_assert(std::is_trivially_copyable_v<T>, "SaveStateWriter::writeArray requires a trivially copyable element type");
        appendBytes(reinterpret_cast<const u8*>(values.data()), sizeof(T) * N);
    }

    void writeVector(const std::vector<u8>& values);

    template<typename T>
    void writeVector(const std::vector<T>& values)
    {
        static_assert(std::is_trivially_copyable_v<T>, "SaveStateWriter::writeVector requires a trivially copyable element type");
        write<u32>(static_cast<u32>(values.size()));
        for(const T& value : values)
            write(value);
    }

    // Takes a copy since std::queue exposes no iteration; the caller's queue is left untouched.
    void writeQueue(std::queue<u32> values);

    const std::vector<u8>& getBuffer() const;

private:
    void appendBytes(const u8* data, size_t size);

    std::vector<u8> buffer;
};
