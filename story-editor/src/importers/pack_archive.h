#ifndef PACK_ARCHIVE_H
#define PACK_ARCHIVE_H

#include <ranges>
#include <string>
#include <vector>
#include <filesystem>

#include "zip.h"
#include "ni_parser.h"
#include "json.hpp"
#include "uuid.h"
#include "i_logger.h"
#include "i_story_db.h"
#include "story_project.h"
#include "sys_lib.h"
#include "nodes_factory.h"

class PackArchive
{
public:
    PackArchive(ILogger &log, NodesFactory &factory);

    std::string OpenImage(const std::string &fileName);

    bool ImportStudioFormat(const std::string &fileName, const std::string &outputDir);

    template<typename Range>
    // requires std::ranges::range<Range>
    void ImportCommercialFormat(const std::string &packFileName, const std::string &libraryBaseDir, Range&& range)
    {
        auto uuid =  Uuid().String();
        std::string outputBaseDir = libraryBaseDir + "/" + uuid;

        auto ext = SysLib::GetFileExtension(packFileName);

        std::filesystem::path importBaseDir;

        // Selon l'importation, l'UUID peut être partiel (à partir d'une carte SD) ou complet (pour un .pk)
        std::string packUuidv4;

        if (ext == "pk")
        {
            // Unzip all before analyze the pack
            Unzip(packFileName, outputBaseDir);

            // Le pack Lunii contient un répertoire contenant l'UUID sans tirets, upper case
            packUuidv4 = SysLib::GetFileName(packFileName);
            SysLib::EraseString(packUuidv4, "." + ext); // on retire l'extension du pack
            importBaseDir =  std::filesystem::path(outputBaseDir) / SysLib::ToUpper(packUuidv4);
        }
        else
        {
            // Ici on a choisi le fichier ni, donc on prend juste le répertoire parent
            importBaseDir = SysLib::GetDirectory(packFileName);
            std::string packDirNameOnly = importBaseDir.filename().string();

            // Ici on va copier le répertoire dans un dossier de travail pour éviter de corrompre le 
            // répertoire d'origine (vu qu'on decipher tout)
            const auto copyOptions = std::filesystem::copy_options::recursive
                | std::filesystem::copy_options::update_existing
                | std::filesystem::copy_options::overwrite_existing
                ;

            std::filesystem::path workingDir = std::filesystem::path(outputBaseDir) / packDirNameOnly;
            std::filesystem::create_directories(workingDir);
            std::filesystem::copy(importBaseDir, workingDir, copyOptions);
                    
            importBaseDir = workingDir; // on travaille là maintenant
            packUuidv4 = SysLib::ToLower(packDirNameOnly); // le répertoire parent est l'UUID mais partiel !! (la fin d'un full UUID)
        }
        
        if (ParseRootFiles(importBaseDir))
        {
            m_log.Log("Parse NI file success");
            ni_dump(&mNiFile);
        }
        else
        {
            m_log.Log("Parse NI file error", true);
        }

        StoryProject proj(m_log);
        proj.New(uuid, libraryBaseDir);
        
        for (auto &info : range)
        {
            bool foundStory = false;

            // full UUIDv4 size
            if (packUuidv4.size() == 32)
            {
                packUuidv4 = normalizeUUID(packUuidv4);
                foundStory = (info.uuid == packUuidv4);
            }
            else
            {
                // Partial UUIDv4, uniquement les 8 derniers caractères
                foundStory = info.uuid.ends_with(packUuidv4);
            }

            if (foundStory)
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

        for (const auto & entry : std::filesystem::directory_iterator(importBaseDir / "rf"))
        {
            if (entry.is_directory())
            {
                std::cout << entry.path() << std::endl;
                DecipherFiles(entry.path().generic_string(), ".bmp");
            }
        }

        for (const auto & entry : std::filesystem::directory_iterator(importBaseDir / "sf"))
        {
            if (entry.is_directory())
            {
                std::cout << entry.path() << std::endl;
                DecipherFiles(entry.path().generic_string(), ".mp3");
            }
        }

        ConvertCommercialFormat(proj, importBaseDir);
    }

    void Unzip(const std::string &filePath, const std::string &parent_dest_dir);
    
    bool ConvertJsonStudioToOst(const std::string &basePath, const std::string &uuid, const std::string &outputDir);

    std::string HexDump(const char *desc, const void *addr, int len);
private:
    ILogger &m_log;
    NodesFactory &m_nodesFactory;
    Zip mZip;
    ni_file_t mNiFile;

    void ConvertCommercialFormat(StoryProject &proj, const std::filesystem::path &importBaseDir);

    bool ParseRootFiles(const std::filesystem::path &root);

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
