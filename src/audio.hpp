#pragma once
#include "message.hpp"
#include <SDL3/SDL.h>

class AudioManager {
public:
    explicit AudioManager(MessageBus& bus);
    ~AudioManager();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

private:
    void playTone(float freq, float duration = 0.09f, float volume = 0.25f);

    SDL_AudioStream* stream_{};
    bool ok_{};
};
