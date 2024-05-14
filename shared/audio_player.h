#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <string>
#include <thread>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>
#include <thread>
#include "thread_safe_queue.h"


struct AudioCommand {
    std::string order;
    std::string filename;
};

class IAudioEvent
{
public:
    virtual ~IAudioEvent() {}

    virtual void EndOfAudio() = 0;
};


class AudioPlayer
{
public:

    AudioPlayer(IAudioEvent &event);
    ~AudioPlayer();

    void Play(const std::string &filename);
    void Stop();
    void Initialize();

private:
    IAudioEvent &m_event;

    std::thread m_audioThread;

    void AudioThread();

};


#endif // AUDIO_PLAYER_H
