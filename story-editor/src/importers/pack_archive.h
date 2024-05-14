#ifndef PACK_ARCHIVE_H
#define PACK_ARCHIVE_H

#include <string>
#include "zip.h"
#include <vector>
#include "ni_parser.h"
#include "json.hpp"
#include "i_logger.h"

class PackArchive
{
public:
    PackArchive(ILogger &log);

    bool Load(const std::string &filePath);
    std::string OpenImage(const std::string &fileName);

    bool ImportStudioFormat(const std::string &fileName, const std::string &outputDir);

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
    void DecipherAll(const std::string &packFileName, const std::string &parent_dest_dir);
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

    // key: resource tag
    // value: resource file name
    std::map<std::string, std::string> m_resources;

    bool ParseNIFile(const std::string &root);


    void DecipherFileOnDisk(const std::string &fileName);
    void DecipherFiles(const std::string &directory, const std::string &suffix);
    std::vector<std::string> FilesToJson(const std::string &type, const uint8_t *data, uint32_t nb_elements);
};

#endif // PACK_ARCHIVE_H
