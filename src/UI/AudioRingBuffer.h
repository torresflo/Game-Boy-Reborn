#pragma once

#include <cstddef>
#include <mutex>
#include <vector>

#include "Common.h"

// Thread-safe FIFO of interleaved stereo s16 samples
class AudioRingBuffer
{
public:
    void push(const std::vector<s16>& samples);
    std::size_t pop(s16* destination, std::size_t maxSamples);
    void clear();

private:
    std::mutex mutex;
    std::vector<s16> buffer;
};
