
#include <functional>
#include <stdio.h>
#include "audio_player.h"

// #define MINIAUDIO_IMPLEMENTATION
// #include "miniaudio.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include "raudio.h"

static int audio_open = 0;
static int next_track = 0;

// static ma_result result;
// static ma_decoder decoder;
// static ma_device_config deviceConfig;
// static ma_device device;

SDL_AudioSpec spec;
    

static ThreadSafeQueue<AudioCommand> g_audioQueue;

// void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
// {
//     ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
//     if (pDecoder == NULL)
//     {
//         return;
//     }

//     if (ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL) != MA_SUCCESS)
//     {
//         g_audioQueue.push({"end", ""});  
//     }


//     (void)pInput;
// }

static Music music;
static Sound sound;

AudioPlayer::AudioPlayer(IAudioEvent &event)
    : m_event(event)
{
    m_audioThread = std::thread( std::bind(&AudioPlayer::AudioThread, this) );
}

void AudioPlayer::Initialize()
{
    InitAudioDevice();
    audio_open = 1;
}

void AudioPlayer::Play(const std::string &filename)
{
    // On coupe le son en cours
    g_audioQueue.clear();
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

    CloseAudioDevice();
}

int AudioPlayer::StartAudio(const std::string &filename)
{

    // music = LoadMusicStream(filename.c_str());
    // PlayMusicStream(music);

    sound = LoadSound(filename.c_str());
    PlaySound(sound);

    /*
    result = ma_decoder_init_file(filename.c_str(), NULL, &decoder);
    if (result != MA_SUCCESS) 
    {
        // FIXME: show error
        return -1;
    }

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = decoder.outputFormat;
    deviceConfig.playback.channels = decoder.outputChannels;
    deviceConfig.sampleRate        = decoder.outputSampleRate;
    deviceConfig.dataCallback      = data_callback;
    deviceConfig.pUserData         = &decoder;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS)
    {
        printf("Failed to open playback device.\n");
        ma_decoder_uninit(&decoder);
        return -3;
    }

    if (ma_device_start(&device) != MA_SUCCESS)
    {
        printf("Failed to start playback device.\n");
        StopAudio();
        return -4;
    }
    */

    return 0;
}

void AudioPlayer::StopAudio()
{
    // ma_device_uninit(&device);
    // ma_decoder_uninit(&decoder);

    StopSound(sound);
    UnloadSound(sound); 

    // StopMusicStream(music);
    // UnloadMusicStream(music);
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
                StartAudio(cmd.filename);
            }
        }
        else if (cmd.order == "end")
        {
            if (state == AUDIO_STATE_WAIT_END)
            {
                state = AUDIO_STATE_WAIT_PLAY;
                StopAudio();
                m_event.EndOfAudio();
            }
        }
    }
}

