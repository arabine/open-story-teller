#include <algorithm>
#include <filesystem>


#include "resource_manager.h"
#include "media_converter.h"
#include "sys_lib.h"
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

void ResourceManager::ConvertResources(const std::filesystem::path &assetsPath, const std::filesystem::path &destAssetsPath)
{
    auto [b, e] = Items();
    for (auto it = b; it != e; ++it)
    {
        std::filesystem::path inputfile = std::filesystem::path(assetsPath / (*it)->file);
        std::filesystem::path outputfile = std::filesystem::path(assetsPath / SysLib::RemoveFileExtension((*it)->file));

        int retCode = 0;
        if (!std::filesystem::exists(outputfile))
        {
            if ((*it)->format == "PNG")
            {
                outputfile += ".qoi"; // FIXME: prendre la config en cours désirée
                retCode = MediaConverter::ImageToQoi(inputfile.generic_string(), outputfile.generic_string());
            }
            else if ((*it)->format == "MP3")
            {
                outputfile += ".wav"; // FIXME: prendre la config en cours désirée
                retCode = MediaConverter::Mp3ToWav(inputfile.generic_string(), outputfile.generic_string());
            }
            else if ((*it)->format == "OGG")
            {
                outputfile += ".wav"; // FIXME: prendre la config en cours désirée
                retCode = MediaConverter::OggToWav(inputfile.generic_string(), outputfile.generic_string());
            }
            else
            {
                // Log("Skipped: " + inputfile + ", unknown format" + outputfile, true);
            }
        }

        if (retCode < 0)
        {
            //m_log.Log("Failed to convert media file " + inputfile + ", error code: " + std::to_string(retCode) + " to: " + outputfile, true);
        }
        else if (retCode == 0)
        {
            // Log("Convertered file: " + inputfile);

            // Conversion success, now copy to output directory if necessary (ie: to an external device)
            if ((destAssetsPath != assetsPath) && (std::filesystem::exists(destAssetsPath)))
            {
                // destination filename
                auto destFile = destAssetsPath / outputfile.filename();

                if (!std::filesystem::exists(destFile))
                {
                    std::filesystem::copy(outputfile, destAssetsPath, std::filesystem::copy_options::overwrite_existing);
                }
            }
        }
    }
}


