#include "SaveStateReader.h"

#include <cstring>

SaveStateReader::SaveStateReader(std::vector<u8> data)
    : buffer(std::move(data))
{
}

bool SaveStateReader::readVector(std::vector<u8>& outValues)
{
    u32 count = 0;
    if(!read(count))
        return false;

    outValues.resize(count);
    return readBytes(outValues.data(), outValues.size());
}

bool SaveStateReader::readQueue(std::queue<u32>& outValues)
{
    outValues = {};

    u32 count = 0;
    if(!read(count))
        return false;

    for(u32 i = 0; i < count; ++i)
    {
        u32 value = 0;
        if(!read(value))
            return false;

        outValues.push(value);
    }
    return true;
}

bool SaveStateReader::readBytes(u8* destination, size_t size)
{
    if(offset + size > buffer.size())
        return false;

    std::memcpy(destination, buffer.data() + offset, size);
    offset += size;
    return true;
}
