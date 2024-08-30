
#include "resource.h"
#include "sys_lib.h"

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

std::string Resource::ImageExtension(const std::string &filename, Resource::ImageFormat prefered_format)
{
    std::string ext = SysLib::GetFileExtension(filename);
    if (prefered_format == Resource::ImageFormat::IMG_FORMAT_QOIF)
    {
        return "qoi";
    }
    
    return ext;
}

std::string Resource::SoundExtension(const std::string &filename, Resource::SoundFormat prefered_format)
{
    std::string ext = SysLib::GetFileExtension(filename);
    if (prefered_format == Resource::SoundFormat::SND_FORMAT_QOAF)
    {
        return "qoa";
    }
    else if (prefered_format == Resource::SoundFormat::SND_FORMAT_WAV)
    {
        return "wav";
    }
    
    return ext;
}
