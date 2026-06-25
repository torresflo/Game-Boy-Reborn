#include "GameBoyAudioStream.h"

#include <algorithm>

#include "AudioProcessingUnit.h"

GameBoyAudioStream::GameBoyAudioStream(AudioRingBuffer& buffer)
    : ringBuffer(buffer)
{
}

void GameBoyAudioStream::initializeStream()
{
    initialize(2, AudioProcessingUnit::OutputSampleRate, {sf::SoundChannel::FrontLeft, sf::SoundChannel::FrontRight});
}

bool GameBoyAudioStream::onGetData(Chunk& data)
{
    std::size_t popped = ringBuffer.pop(chunkScratchBuffer.data(), ChunkSampleCount);

    if(popped < ChunkSampleCount)
        std::fill(chunkScratchBuffer.begin() + static_cast<std::ptrdiff_t>(popped), chunkScratchBuffer.end(), s16{0});

    data.samples = chunkScratchBuffer.data();
    data.sampleCount = ChunkSampleCount;
    return true;
}

void GameBoyAudioStream::onSeek(sf::Time /*timeOffset*/)
{
    //Live stream; nothing to seek to.
}
