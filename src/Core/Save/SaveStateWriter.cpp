#include "SaveStateWriter.h"

void SaveStateWriter::writeVector(const std::vector<u8>& values)
{
    write<u32>(static_cast<u32>(values.size()));
    appendBytes(values.data(), values.size());
}

void SaveStateWriter::writeQueue(std::queue<u32> values)
{
    write<u32>(static_cast<u32>(values.size()));
    while(!values.empty())
    {
        write<u32>(values.front());
        values.pop();
    }
}

const std::vector<u8>& SaveStateWriter::getBuffer() const
{
    return buffer;
}

void SaveStateWriter::appendBytes(const u8* data, size_t size)
{
    buffer.insert(buffer.end(), data, data + size);
}
