#include "AudioRingBuffer.h"

#include <algorithm>

void AudioRingBuffer::push(const std::vector<s16>& samples)
{
    std::lock_guard<std::mutex> lock(mutex);
    buffer.insert(buffer.end(), samples.begin(), samples.end());
}

std::size_t AudioRingBuffer::pop(s16* destination, std::size_t maxSamples)
{
    std::lock_guard<std::mutex> lock(mutex);

    std::size_t count = std::min(maxSamples, buffer.size());
    std::copy(buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(count), destination);
    buffer.erase(buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(count));

    return count;
}

void AudioRingBuffer::clear()
{
    std::lock_guard<std::mutex> lock(mutex);
    buffer.clear();
}
