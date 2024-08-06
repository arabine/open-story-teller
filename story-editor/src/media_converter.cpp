#include "media_converter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"


#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_LINEAR
#include "stb_image.h"

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

#define QOI_IMPLEMENTATION
#include "qoi.h"


static int qoi_encode_and_write(const char *filename, const void *data, const qoi_desc *desc) {
	FILE *f = fopen(filename, "wb");
	int size;
	void *encoded;

	if (!f) {
		return 0;
	}

	encoded = qoi_encode(data, desc, &size);
	if (!encoded) {
		fclose(f);
		return 0;
	}

	fwrite(encoded, 1, size, f);
	fclose(f);

	free(encoded);
	return size;
}

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


        int encoded = qoi_encode_and_write(outputFileName.c_str(), pixels, &desc);
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


// Function to read the entire contents of a file into memory
static unsigned char* read_entire_file(const char* filename, int* length) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char* buffer = (unsigned char*)malloc(size);
    if (!buffer) {
        fclose(f);
        return NULL;
    }
    fread(buffer, 1, size, f);
    fclose(f);
    if (length) *length = (int)size;
    return buffer;
}

int MediaConverter::OggToWav(const std::string &inputFileName, const std::string &outputFileName)
{

    int ogg_length;
    unsigned char* ogg_data = read_entire_file(inputFileName.c_str(), &ogg_length);
    if (!ogg_data) {
        std::cout << "Failed to read input file " << inputFileName << std::endl;
        return 1;
    }

    int channels, sample_rate;
    short* samples = NULL;
    // (const uint8 *mem, int len, int *channels, int *sample_rate, short **output
    int data_len = stb_vorbis_decode_memory(ogg_data, ogg_length, &channels, &sample_rate, &samples);
    if (!samples) {
        std::cout <<"Failed to decode OGG file " << inputFileName << std::endl;
        free(ogg_data);
        return 1;
    }

    // Write WAV file header
    FILE* wav_file = fopen(outputFileName.c_str(), "wb");
    if (!wav_file) {
        std::cout << "Failed to create output file " << outputFileName << std::endl;
        free(ogg_data);
        free(samples);
        return 1;
    }
    fwrite("RIFF", 1, 4, wav_file);
    int total_size = 36 + channels * (sample_rate * sizeof(short));
    fwrite(&total_size, 4, 1, wav_file);
    fwrite("WAVEfmt ", 1, 8, wav_file);
    int format_size = 16;
    fwrite(&format_size, 4, 1, wav_file);
    short format_type = 1; // PCM
    fwrite(&format_type, 2, 1, wav_file);
    fwrite(&channels, 2, 1, wav_file);
    fwrite(&sample_rate, 4, 1, wav_file);
    int byte_rate = sample_rate * channels * sizeof(short);
    fwrite(&byte_rate, 4, 1, wav_file);
    short block_align = channels * sizeof(short);
    fwrite(&block_align, 2, 1, wav_file);
    short bits_per_sample = 16;
    fwrite(&bits_per_sample, 2, 1, wav_file);
    fwrite("data", 1, 4, wav_file);
   // int data_size = channels * (sample_rate * sizeof(short));

    int data_size = data_len * channels * 2; // *2 to make in bytes
    fwrite(&data_size, 4, 1, wav_file);

    // Write sample data
    fwrite(samples, sizeof(short), data_len, wav_file);

    fclose(wav_file);

    free(ogg_data);
    free(samples);

    return cSuccess;
}
