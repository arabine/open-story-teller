

#include <vector>
#include <iostream>
#include <string>
#include <cstdint>

#include "AudioFile.h"

#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#include "minimp3_ex.h"

std::string GetSuffix(const std::string &path)
{
    return path.substr(path.find_last_of(".") + 1);
}

std::string GetFullBaseName(const std::string &path)
{
    std::string suffix = GetSuffix(path);
    std::string basename = path;
    basename.erase(path.size()-(suffix.size()+1));
    return basename;
}

void PrintHelp(const char *exeName)
{
    std::cout << exeName << " - WAV from/to SVG converter" << std::endl;
    std::cout << "Usage: " << exeName << " myfile.[wav|svg]" << std::endl;
}


void WavInspector(const std::string &inputFileName)
{

    AudioFile<int16_t> audioFile;
    bool loadedOK = audioFile.load(inputFileName);

    if (loadedOK)
    {
        int samples = audioFile.getNumSamplesPerChannel();


        audioFile.printSummary();
        for (int k = 0; k < audioFile.getNumChannels(); k++)
        {
            std::cout << "Found channel: " << k << std::endl;
            for (int i = 0; i < 10; i++)
            {
                int16_t sampleRaw = audioFile.samples[k][i];
                std::cout << (int)sampleRaw << std::endl;
            }
        }

    }
}

int Mp3ToWav(const std::string &inputfile, const std::string &outputfile)
{
    int ret = -1;

    FILE* fp = fopen(inputfile.c_str(), "rb");
    if (!fp) {
        printf("Could not open file\n");
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long mp3_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    uint8_t* mp3_data = new uint8_t[mp3_size];
    if (fread(mp3_data, 1, mp3_size, fp) == mp3_size)
    {

        // MP3 to WAV
        mp3dec_t mp3d;
        mp3dec_file_info_t info;
        mp3dec_load_buf(&mp3d, mp3_data, mp3_size, &info, NULL, NULL);

        AudioFile<int16_t> audioFile;

        audioFile.initializeAudioBuffer(info.buffer, info.samples, info.channels);

        audioFile.save(outputfile, AudioFileFormat::Wave);

        free(info.buffer);
    }
    else
    {
        printf("Could not read file\n");
    }

    delete[] mp3_data;
    fclose(fp);

    return ret;
}


int main(int argc, char *argv[])
{
    int ret = 0;
    if (argc > 1)
    {
        std::cout << GetFullBaseName(argv[1]) << std::endl;

        std::string baseName = GetFullBaseName(argv[1]);
        std::string suffix = GetSuffix(argv[1]);

        if (suffix == "wav")
        {
            WavInspector(argv[1]);
        }
        else
        {
            ret = -1;
        }

    }
    if (argc > 2)
    {
        std::string suffix = GetSuffix(argv[1]);
        if (suffix == "mp3")
        {

            Mp3ToWav(argv[1], argv[2]);
        }
        else
        {
            ret = -1;
        }
    }
    else
    {
        ret = -2;
    }

    if (ret != 0)
    {
        PrintHelp(argv[0]);
    }

    return ret;
}
