#pragma once

#include <array>

#include <SFML/Audio/SoundStream.hpp>

#include "Common.h"
#include "AudioRingBuffer.h"

// Pulls decoded PCM samples from an AudioRingBuffer and feeds them to SFML's
// audio backend. onGetData/onSeek run on SFML's internal audio thread, which
// is what makes AudioRingBuffer's thread safety necessary.
class GameBoyAudioStream : public sf::SoundStream
{
public:
    explicit GameBoyAudioStream(AudioRingBuffer& buffer);

    void initializeStream();

protected:
    bool onGetData(Chunk& data) override;
    void onSeek(sf::Time timeOffset) override;

private:
    static constexpr std::size_t ChunkSampleCount = 2048; // ~46ms of stereo audio at 44100Hz

    AudioRingBuffer& ringBuffer;
    std::array<s16, ChunkSampleCount> chunkScratchBuffer{};
};
