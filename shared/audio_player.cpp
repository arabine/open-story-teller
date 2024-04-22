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

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"


#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <functional>
#include <stdio.h>
#include "audio_player.h"

// #define MA_NO_LIBOPUS
// #include "miniaudio_libvorbis.h"
// #include "miniaudio_libopus.h"

#include <stdio.h>

#if 0

static ma_result ma_decoding_backend_init__libvorbis(void* pUserData, ma_read_proc onRead, ma_seek_proc onSeek, ma_tell_proc onTell, void* pReadSeekTellUserData, const ma_decoding_backend_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_data_source** ppBackend)
{
    ma_result result;
    ma_libvorbis* pVorbis;

    (void)pUserData;

    pVorbis = (ma_libvorbis*)ma_malloc(sizeof(*pVorbis), pAllocationCallbacks);
    if (pVorbis == NULL) {
        return MA_OUT_OF_MEMORY;
    }

    result = ma_libvorbis_init(onRead, onSeek, onTell, pReadSeekTellUserData, pConfig, pAllocationCallbacks, pVorbis);
    if (result != MA_SUCCESS) {
        ma_free(pVorbis, pAllocationCallbacks);
        return result;
    }

    *ppBackend = pVorbis;

    return MA_SUCCESS;
}

static ma_result ma_decoding_backend_init_file__libvorbis(void* pUserData, const char* pFilePath, const ma_decoding_backend_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_data_source** ppBackend)
{
    ma_result result;
    ma_libvorbis* pVorbis;

    (void)pUserData;

    pVorbis = (ma_libvorbis*)ma_malloc(sizeof(*pVorbis), pAllocationCallbacks);
    if (pVorbis == NULL) {
        return MA_OUT_OF_MEMORY;
    }

    result = ma_libvorbis_init_file(pFilePath, pConfig, pAllocationCallbacks, pVorbis);
    if (result != MA_SUCCESS) {
        ma_free(pVorbis, pAllocationCallbacks);
        return result;
    }

    *ppBackend = pVorbis;

    return MA_SUCCESS;
}

static void ma_decoding_backend_uninit__libvorbis(void* pUserData, ma_data_source* pBackend, const ma_allocation_callbacks* pAllocationCallbacks)
{
    ma_libvorbis* pVorbis = (ma_libvorbis*)pBackend;

    (void)pUserData;

    ma_libvorbis_uninit(pVorbis, pAllocationCallbacks);
    ma_free(pVorbis, pAllocationCallbacks);
}

static ma_result ma_decoding_backend_get_channel_map__libvorbis(void* pUserData, ma_data_source* pBackend, ma_channel* pChannelMap, size_t channelMapCap)
{
    ma_libvorbis* pVorbis = (ma_libvorbis*)pBackend;

    (void)pUserData;

    return ma_libvorbis_get_data_format(pVorbis, NULL, NULL, NULL, pChannelMap, channelMapCap);
}

static ma_decoding_backend_vtable g_ma_decoding_backend_vtable_libvorbis =
{
    ma_decoding_backend_init__libvorbis,
    ma_decoding_backend_init_file__libvorbis,
    NULL, /* onInitFileW() */
    NULL, /* onInitMemory() */
    ma_decoding_backend_uninit__libvorbis
};



static ma_result ma_decoding_backend_init__libopus(void* pUserData, ma_read_proc onRead, ma_seek_proc onSeek, ma_tell_proc onTell, void* pReadSeekTellUserData, const ma_decoding_backend_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_data_source** ppBackend)
{
    ma_result result;
    ma_libopus* pOpus;

    (void)pUserData;

    pOpus = (ma_libopus*)ma_malloc(sizeof(*pOpus), pAllocationCallbacks);
    if (pOpus == NULL) {
        return MA_OUT_OF_MEMORY;
    }

    result = ma_libopus_init(onRead, onSeek, onTell, pReadSeekTellUserData, pConfig, pAllocationCallbacks, pOpus);
    if (result != MA_SUCCESS) {
        ma_free(pOpus, pAllocationCallbacks);
        return result;
    }

    *ppBackend = pOpus;

    return MA_SUCCESS;
}

static ma_result ma_decoding_backend_init_file__libopus(void* pUserData, const char* pFilePath, const ma_decoding_backend_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_data_source** ppBackend)
{
    ma_result result;
    ma_libopus* pOpus;

    (void)pUserData;

    pOpus = (ma_libopus*)ma_malloc(sizeof(*pOpus), pAllocationCallbacks);
    if (pOpus == NULL) {
        return MA_OUT_OF_MEMORY;
    }

    result = ma_libopus_init_file(pFilePath, pConfig, pAllocationCallbacks, pOpus);
    if (result != MA_SUCCESS) {
        ma_free(pOpus, pAllocationCallbacks);
        return result;
    }

    *ppBackend = pOpus;

    return MA_SUCCESS;
}

static void ma_decoding_backend_uninit__libopus(void* pUserData, ma_data_source* pBackend, const ma_allocation_callbacks* pAllocationCallbacks)
{
    ma_libopus* pOpus = (ma_libopus*)pBackend;

    (void)pUserData;

    ma_libopus_uninit(pOpus, pAllocationCallbacks);
    ma_free(pOpus, pAllocationCallbacks);
}

static ma_result ma_decoding_backend_get_channel_map__libopus(void* pUserData, ma_data_source* pBackend, ma_channel* pChannelMap, size_t channelMapCap)
{
    ma_libopus* pOpus = (ma_libopus*)pBackend;

    (void)pUserData;

    return ma_libopus_get_data_format(pOpus, NULL, NULL, NULL, pChannelMap, channelMapCap);
}

static ma_decoding_backend_vtable g_ma_decoding_backend_vtable_libopus =
{
    ma_decoding_backend_init__libopus,
    ma_decoding_backend_init_file__libopus,
    NULL, /* onInitFileW() */
    NULL, /* onInitMemory() */
    ma_decoding_backend_uninit__libopus
};


#endif

// void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
// {
//     ma_data_source* pDataSource = (ma_data_source*)pDevice->pUserData;
//     if (pDataSource == NULL) {
//         return;
//     }

//     ma_data_source_read_pcm_frames(pDataSource, pOutput, frameCount, NULL);

//     (void)pInput;
// }


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
    short *output = NULL;

    result = ma_decoder_init_file(filename, NULL, &decoder);
    if (result != MA_SUCCESS)
    {
        printf("Could not load file: %s\n", filename);
        
        // Try OGG (FIXME: plus tard, utiliser l'extension pour choisir le décodeur)
        int channels = 0, sample_rate = 0;
        
        int sample_count = stb_vorbis_decode_filename("chemin/vers/fichier.ogg", &channels, &sample_rate, &output);
        if (sample_count == -1)
        {
            printf("Erreur de décodage du fichier Vorbis\n");
            return -2;
        }

        ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
        deviceConfig.playback.format   = ma_format_s16;
        deviceConfig.playback.channels = channels;
        deviceConfig.sampleRate        = sample_rate;
        deviceConfig.dataCallback      = data_callback;
        deviceConfig.pUserData         = &decoder;        
    }
    else
    {
        deviceConfig = ma_device_config_init(ma_device_type_playback);
        deviceConfig.playback.format   = decoder.outputFormat;
        deviceConfig.playback.channels = decoder.outputChannels;
        deviceConfig.sampleRate        = decoder.outputSampleRate;
        deviceConfig.dataCallback      = data_callback;
        deviceConfig.pUserData         = &decoder;
    }

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        ma_decoder_uninit(&decoder);
        if (output != NULL)
        {
            free(output);
        }
        return -3;
    }

    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
        if (output != NULL)
        {
            free(output);
        }
        return -4;
    }

    if (output != NULL)
    {
        free(output);
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

