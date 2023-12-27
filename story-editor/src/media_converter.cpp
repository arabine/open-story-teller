#include "media_converter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_LINEAR
#include "stb_image.h"

#define QOI_IMPLEMENTATION
#undef QOI_NO_STDIO
#include "qoi.h"

//#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

MediaConverter::MediaConverter()
{

}


int MediaConverter::ImageToQoi(const std::string &inputFileName, const std::string &outputFileName)
{
    void *pixels = NULL;
    int w = 0;
    int h = 0;
    int channels = 0;

    if(!stbi_info(inputFileName.c_str(), &w, &h, &channels))
    {
        return cErrorBadInputFileFormat;
    }

    // Force all odd encodings to be RGBA
    if(channels != 3) {
        channels = 4;
    }

    pixels = (void *)stbi_load(inputFileName.c_str(), &w, &h, NULL, channels);

    if (pixels != NULL)
    {
        qoi_desc desc;
        desc.channels = channels;
        desc.colorspace = QOI_SRGB;
        desc.width = w;
        desc.height = h;

        int encoded = qoi_write(outputFileName.c_str(), pixels, &desc);
        free(pixels);

        if (!encoded)
        {
            return cErrorCannotWriteOrEncodeOutputFile;
        }
    }
    else
    {
        return cErrorCannotDecodeInputFile;
    }

    return cSuccess;
}

void fwrite_u32_le(unsigned int v, FILE *fh) {
    unsigned char buf[sizeof(unsigned int)];
    buf[0] = 0xff & (v      );
    buf[1] = 0xff & (v >>  8);
    buf[2] = 0xff & (v >> 16);
    buf[3] = 0xff & (v >> 24);
    (void) fwrite(buf, sizeof(unsigned int), 1, fh);
}

void fwrite_u16_le(unsigned short v, FILE *fh) {
    unsigned char buf[sizeof(unsigned short)];
    buf[0] = 0xff & (v      );
    buf[1] = 0xff & (v >>  8);
    (void) fwrite(buf, sizeof(unsigned short), 1, fh);
}

int MediaConverter::WavWrite(const std::string &path, short *sample_data, MediaInfo &desc)
{
    unsigned int data_size = desc.samples * desc.channels * sizeof(short);
    unsigned int samplerate = desc.samplerate;
    short bits_per_sample = 16;
    short channels = desc.channels;

    /* Lifted from https://www.jonolick.com/code.html - public domain
    Made endian agnostic using qoaconv_fwrite() */
    FILE *fh = fopen(path.c_str(), "wb");

    if (fh != NULL)
    {
        fwrite("RIFF", 1, 4, fh);
        fwrite_u32_le(data_size + 44 - 8, fh);
        fwrite("WAVEfmt \x10\x00\x00\x00\x01\x00", 1, 14, fh);
        fwrite_u16_le(channels, fh);
        fwrite_u32_le(samplerate, fh);
        fwrite_u32_le(channels * samplerate * bits_per_sample/8, fh);
        fwrite_u16_le(channels * bits_per_sample/8, fh);
        fwrite_u16_le(bits_per_sample, fh);
        fwrite("data", 1, 4, fh);
        fwrite_u32_le(data_size, fh);
        fwrite((void*)sample_data, data_size, 1, fh);
        fclose(fh);
        return data_size  + 44 - 8;
    }
    return cErrorCannotWriteOrEncodeOutputFile;

}


short *MediaConverter::Mp3Read(const std::string &path, MediaInfo &desc)
{
    drmp3_uint64 samples = 0;

    drmp3_config mp3 = {
        .channels = 2,
        .sampleRate = 44100
    };
    short* sample_data = drmp3_open_file_and_read_pcm_frames_s16(path.c_str(), &mp3, &samples, NULL);

    desc.samplerate = mp3.sampleRate;
    desc.channels = mp3.channels;
    desc.samples = samples;

    return sample_data;
}

int MediaConverter::Mp3ToWav(const std::string &inputFileName, const std::string &outputFileName)
{
    MediaInfo media;
    short *sample_data = NULL;

    sample_data = Mp3Read(inputFileName, media);

    if (sample_data != NULL)
    {

        int bytes_written = WavWrite(outputFileName, sample_data, media);

        free(sample_data);

        if (bytes_written < 0)
        {
            return bytes_written; // error
        }
    }
    else
    {
        return cErrorBadInputFileFormat;
    }

    return cSuccess;
}
