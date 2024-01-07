/*
Demonstrates how to load a sound file and play it back using the low-level API.

The low-level API uses a callback to deliver audio between the application and miniaudio for playback or recording. When
in playback mode, as in this example, the application sends raw audio data to miniaudio which is then played back through
the default playback device as defined by the operating system.

This example uses the `ma_decoder` API to load a sound and play it back. The decoder is entirely decoupled from the
device and can be used independently of it. This example only plays back a single sound file, but it's possible to play
back multiple files by simple loading multiple decoders and mixing them (do not create multiple devices to do this). See
the simple_mixing example for how best to do this.
*/
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <functional>
#include <stdio.h>
#include "audio_player.h"


static ThreadSafeQueue<AudioCommand> g_audioQueue;
static ma_decoder decoder;
static ma_device device;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    ma_uint64 framesRead;
    ma_result result =  ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, &framesRead);


    if (result == MA_AT_END) {
        g_audioQueue.push({"end", ""});
    }
    //    if (framesRead < frameCount) {
    //        // Reached the end.
    //        ma_event_signal(&g_stopEvent);
    //    }

    (void)pInput;
}


static int miniaudio_play(const char* filename)
{
    ma_result result;
    ma_device_config deviceConfig;

    result = ma_decoder_init_file(filename, NULL, &decoder);
    if (result != MA_SUCCESS) {
        printf("Could not load file: %s\n", filename);
        return -2;
    }

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = decoder.outputFormat;
    deviceConfig.playback.channels = decoder.outputChannels;
    deviceConfig.sampleRate        = decoder.outputSampleRate;
    deviceConfig.dataCallback      = data_callback;
    deviceConfig.pUserData         = &decoder;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        ma_decoder_uninit(&decoder);
        return -3;
    }

    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
        return -4;
    }

    return 0;
}

AudioPlayer::AudioPlayer(IAudioEvent &event)
    : m_event(event)
{
    m_audioThread = std::thread( std::bind(&AudioPlayer::AudioThread, this) );
}

void AudioPlayer::Play(const std::string &filename)
{
    g_audioQueue.push({"play", filename});
}

AudioPlayer::~AudioPlayer()
{
    // Quit audio thread
    g_audioQueue.push({"quit", ""});
    if (m_audioThread.joinable())
    {
        m_audioThread.join();
    }
}

void AudioPlayer::CloseAudio()
{
    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);
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

        if (cmd.order == "quit") {
            return;
        }
        else if (cmd.order == "play")
        {
            if (state == AUDIO_STATE_WAIT_PLAY)
            {
                state = AUDIO_STATE_WAIT_END;
                miniaudio_play(cmd.filename.c_str());
            }
        }
        else if (cmd.order == "end")
        {
            if (state == AUDIO_STATE_WAIT_END)
            {
                state = AUDIO_STATE_WAIT_PLAY;
                CloseAudio();
                m_event.EndOfAudio();
            }
        }
    }
}

