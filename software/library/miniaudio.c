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

#include <stdio.h>


ma_event g_stopEvent; /* <-- Signaled by the audio thread, waited on by the main thread. */

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    ma_uint64 framesRead;
    ma_result result =  ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, &framesRead);
    
    if (framesRead < frameCount) {
        // Reached the end.
        ma_event_signal(&g_stopEvent);
    }

    (void)pInput;
}

int miniaudio_play(const char* filename)
{
    ma_result result;
    ma_decoder decoder;
    ma_device_config deviceConfig;
    ma_device device;

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
    
    ma_event_init(&g_stopEvent);

    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
        return -4;
    }

    printf("Wait untile end...\n");
   // getchar();
    ma_event_wait(&g_stopEvent);
    printf("End!\n");

    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);

    return 0;
}

