
#include "resource.h"

std::string Resource::ImageFormatToString(ImageFormat format)
{
    std::string text = "SAME";
    switch (format)
    {
        case IMG_FORMAT_QOIF: 
            text = "QOIF";
            break;
    }
    return text;
}

std::string Resource::SoundFormatToString(SoundFormat format)
{
    std::string text = "SAME";
    switch (format)
    {
        case SND_FORMAT_WAV: 
            text = "WAV";
            break;
        case SND_FORMAT_QOAF: 
            text = "QOAF";
            break;
    }
    return text;
}

