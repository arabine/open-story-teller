

#include <vector>
#include <iostream>
#include <string>
#include <cstdint>

#include "AudioFile.h"

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


int main(int argc, char *argv[])
{
    int ret = 0;
    if (argc > 1)
    {
        std::cout << GetFullBaseName(argv[1]) << std::endl;

        std::string baseName = GetFullBaseName(argv[1]);
        std::string suffix = GetSuffix(argv[1]);

        if(suffix == "wav")
        {
            WavInspector(argv[1]);
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
