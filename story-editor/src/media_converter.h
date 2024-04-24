#ifndef MEDIA_CONVERTER_H
#define MEDIA_CONVERTER_H

#include <string>
#include <cstdint>

class MediaConverter
{
public:
    static const int cSuccess = 0;
    static const int cErrorBadInputFileFormat = -1;
    static const int cErrorCannotDecodeInputFile = -2;
    static const int cErrorCannotWriteOrEncodeOutputFile = -3;


    struct MediaInfo {
        uint32_t samplerate;
        uint32_t channels;
        uint64_t samples;
    };

    MediaConverter();
    static int ImageToQoi(const std::string &inputFileName, const std::string &outputFileName);
    static int Mp3ToWav(const std::string &inputFileName, const std::string &outputFileName);
    static int OggToWav(const std::string &inputFileName, const std::string &outputFileName);
private:
    static short *Mp3Read(const std::string &path, MediaInfo &desc);
    static int WavWrite(const std::string &path, short *sample_data, MediaInfo &desc);
};

#endif // MEDIA_CONVERTER_H
