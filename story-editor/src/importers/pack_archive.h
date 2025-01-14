#ifndef PACK_ARCHIVE_H
#define PACK_ARCHIVE_H

#include <ranges>
#include <string>
#include <vector>

#include "zip.h"
#include "ni_parser.h"
#include "json.hpp"
#include "uuid.h"
#include "i_logger.h"
#include "i_story_db.h"
#include "story_project.h"

class PackArchive
{
public:
    PackArchive(ILogger &log);

    bool LoadNiFile(const std::string &filePath);
    std::string OpenImage(const std::string &fileName);

    bool ImportStudioFormat(const std::string &fileName, const std::string &outputDir);

    template<typename Range>
    // requires std::ranges::range<Range>
    void ImportCommercialFormat(const std::string &packFileName, const std::string &outputDir, Range&& range)
    {
        auto uuid =  Uuid().String();
        std::string basePath = outputDir + "/" + uuid;

        Unzip(packFileName, basePath);
        LoadNiFile(packFileName);

        StoryProject proj(m_log);
        proj.New(uuid, outputDir);

        auto packUuidv4 = normalizeUUID(mPackName);
        for (auto &info : range)
        {
            std::cout << info.uuid << std::endl;
            if (info.uuid == packUuidv4)
            {
                m_log.Log("Found commercial story: " + info.title);
                proj.SetName(info.title);
                proj.SetDescription(info.description);

                // FIXME: download image and sound
                proj.SetTitleImage(info.image_url);
                proj.SetTitleSound(info.sound);
                break;
            }
        }

        std::string path = basePath + "/" + mPackName + "/rf";
        for (const auto & entry : std::filesystem::directory_iterator(path))
        {
            if (entry.is_directory())
            {
                std::cout << entry.path() << std::endl;
                DecipherFiles(entry.path().generic_string(), ".bmp");
            }
        }

        path = basePath + "/" + mPackName + "/sf";
        for (const auto & entry : std::filesystem::directory_iterator(path))
        {
            if (entry.is_directory())
            {
                std::cout << entry.path() << std::endl;
                DecipherFiles(entry.path().generic_string(), ".mp3");
            }
        }

        ConvertCommercialFormat(proj, basePath);
    }

    std::string CurrentImage();
    std::string CurrentSound();
    std::string CurrentSoundName();
    bool AutoPlay();
    void OkButton();
    bool HasImage();
    std::vector<std::string> GetImages();
    std::string GetImage(const std::string &fileName);
    bool IsRoot() const;
    bool IsWheelEnabled() const;
    void Next();
    void Previous();
    void Unzip(const std::string &filePath, const std::string &parent_dest_dir);
    
    bool ConvertJsonStudioToOst(const std::string &basePath, const std::string &uuid, const std::string &outputDir);

    std::string HexDump(const char *desc, const void *addr, int len);
private:
    ILogger &m_log;
    Zip mZip;
    std::string mPackName;
    uint32_t mCurrentNodeId = 0;
    uint32_t mNodeIdForChoice = 0;
    node_info_t mNodeForChoice;
    node_info_t mCurrentNode;
    ni_file_t mNiFile;

    void ConvertCommercialFormat(StoryProject &proj, const std::string &basePath);

    bool ParseNIFile(const std::string &root);

    // Convertit un UUID de type "3ADE540306254FFFA22B9025AC3678D9"
    // en standard : 3ade5403-0625-4fff-a22b-9025ac3678d9
    std::string normalizeUUID(const std::string& uuid) {
        // Check if the input length is correct for a UUID without dashes
        if (uuid.length() != 32) {
            throw std::invalid_argument("Invalid UUID length");
        }

        // Convert to lowercase
        std::string lowerUuid = uuid;
        std::transform(lowerUuid.begin(), lowerUuid.end(), lowerUuid.begin(), ::tolower);

        // Insert dashes at appropriate positions
        std::ostringstream oss;
        oss << lowerUuid.substr(0, 8) << '-'
            << lowerUuid.substr(8, 4) << '-'
            << lowerUuid.substr(12, 4) << '-'
            << lowerUuid.substr(16, 4) << '-'
            << lowerUuid.substr(20);

        return oss.str();
    }

    void DecipherFileOnDisk(const std::string &fileName);
    void DecipherFiles(const std::string &directory, const std::string &suffix);
    std::vector<std::string> FilesInMemory(const std::string &type, const uint8_t *data, uint32_t nb_elements);
};

#endif // PACK_ARCHIVE_H
