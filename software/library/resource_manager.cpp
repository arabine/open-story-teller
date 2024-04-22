#include "resource_manager.h"
#include <algorithm>

struct Media {
    std::string type;
    std::string format;
};

static const std::unordered_map<std::string, Media> mediaTypes {
    {".mp3", {"sound", "MP3"}},
    {".wav", {"sound","WAV"}},
    {".qoi", {"sound","QOI"}},
    {".ogg", {"sound","OGG"}},
    {".jpg", {"image", "JPEG"}},
    {".jpeg",{"image", "JPEG"}},
    {".png", {"image","PNG"}},
    {".bmp", {"image","BMP"}},

};

std::string ResourceManager::ExtentionInfo(std::string extension, int info_type)
{
    std::string lowerExtension = extension;
    std::transform(lowerExtension.begin(), lowerExtension.end(), lowerExtension.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    auto it = mediaTypes.find(lowerExtension);
    if (it != mediaTypes.end()) {
        return info_type == 0 ? it->second.format : it->second.type;
    } else {
        return "UNKNOWN";
    }
}

