
#include <functional>
#include <stdio.h>
#include "audio_player.h"

#include <SDL3_mixer/SDL_mixer.h>
#include <stdio.h>


static int audio_open = 0;
static Mix_Music *music = NULL;
static int next_track = 0;

SDL_AudioSpec spec;
    

static ThreadSafeQueue<AudioCommand> g_audioQueue;


void music_finished()
{
    Mix_FreeMusic(music);
    music = NULL;
    g_audioQueue.push({"end", ""});    
}


AudioPlayer::AudioPlayer(IAudioEvent &event)
    : m_event(event)
{
    m_audioThread = std::thread( std::bind(&AudioPlayer::AudioThread, this) );
}

void AudioPlayer::Initialize()
{
    /* Initialize variables */
    spec.freq = MIX_DEFAULT_FREQUENCY;
    spec.format = MIX_DEFAULT_FORMAT;
    spec.channels = MIX_DEFAULT_CHANNELS;

    if (Mix_Init(MIX_INIT_FLAC | MIX_INIT_MP3 | MIX_INIT_OGG) < 0) {
        return ;
    }

    if (Mix_OpenAudio(0, &spec) < 0) {
        SDL_Log("Couldn't open audio: %s\n", SDL_GetError());
        return;
    } else {
        Mix_QuerySpec(&spec.freq, &spec.format, &spec.channels);
        SDL_Log("Opened audio at %d Hz %d bit%s %s audio buffer\n", spec.freq,
            (spec.format&0xFF),
            (SDL_AUDIO_ISFLOAT(spec.format) ? " (float)" : ""),
            (spec.channels > 2) ? "surround" : (spec.channels > 1) ? "stereo" : "mono");
    }
    audio_open = 1;

    Mix_HookMusicFinished(music_finished);

}

void AudioPlayer::Play(const std::string &filename)
{
    g_audioQueue.push({"play", filename});
}

AudioPlayer::~AudioPlayer()
{
    // Quit audio thread
    g_audioQueue.clear();
    g_audioQueue.push({"quit", ""});
    if (m_audioThread.joinable())
    {
        m_audioThread.join();
    }
}

 void AudioPlayer::Stop()
 {
    g_audioQueue.clear();
    g_audioQueue.push({"end", ""});
 }


#define AUDIO_STATE_WAIT_PLAY  1
#define AUDIO_STATE_WAIT_END  2

void AudioPlayer::AudioThread()
{
    int state = AUDIO_STATE_WAIT_PLAY;
    for (;;)
    {
        auto cmd = g_audioQueue.front();
        g_audioQueue.pop();

        if (cmd.order == "quit")
        {
            return;
        }
        else if (cmd.order == "play")
        {
            if (state == AUDIO_STATE_WAIT_PLAY)
            {
                state = AUDIO_STATE_WAIT_END;
                music = Mix_LoadMUS(cmd.filename.c_str());

                if (music)
                {
                    Mix_PlayMusic(music, 0);
                }
            }
        }
        else if (cmd.order == "end")
        {
            if (state == AUDIO_STATE_WAIT_END)
            {
                state = AUDIO_STATE_WAIT_PLAY;
                Mix_HaltMusic();
                Mix_FreeMusic(music);
                m_event.EndOfAudio();
            }
        }
    }
}

