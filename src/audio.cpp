#include "audio.hpp"
#include <cmath>
#include <vector>

AudioManager::AudioManager(MessageBus& bus)
{
    SDL_AudioSpec spec{};
    spec.format   = SDL_AUDIO_F32;
    spec.channels = 1;
    spec.freq     = 48000;
    stream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                        &spec, nullptr, nullptr);
    if (stream_)
        ok_ = SDL_ResumeAudioStreamDevice(stream_);

    bus.subscribe(Event::WallBounce, [this](Event) { playTone(300.0f); });
    bus.subscribe(Event::PaddleHit,  [this](Event) { playTone(440.0f); });
    bus.subscribe(Event::Score,      [this](Event) { playTone(200.0f, 0.2f); });
    bus.subscribe(Event::Win,        [this](Event) { playTone(880.0f, 0.4f); });
}

AudioManager::~AudioManager()
{
    if (stream_) SDL_DestroyAudioStream(stream_);
}

void AudioManager::playTone(float freq, float duration, float volume)
{
    if (!ok_) return;

    const int sr    = 48000;
    const int count = static_cast<int>(duration * static_cast<float>(sr));
    std::vector<float> samples(static_cast<std::size_t>(count));

    for (int i = 0; i < count; ++i) {
        const float t   = static_cast<float>(i) / static_cast<float>(sr);
        const float env = 1.0f - t / duration;
        samples[static_cast<std::size_t>(i)] =
            std::sinf(2.0f * 3.14159265f * freq * t) * env * volume;
    }

    SDL_PutAudioStreamData(stream_, samples.data(),
                           static_cast<int>(samples.size() * sizeof(float)));
}
