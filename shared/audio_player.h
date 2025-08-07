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
#include "i_audio_event.h"

struct AudioCommand {
    std::string order;
    std::string filename;
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

    int StartAudio(const std::string &filename);
    void StopAudio();

};


#endif // AUDIO_PLAYER_H
