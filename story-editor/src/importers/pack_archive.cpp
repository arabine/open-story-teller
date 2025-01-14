#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>


#include "pack_archive.h"
#include "ni_parser.h"
#include "json.hpp"
#include "serializers.h"
#include "story_project.h"
#include "resource_manager.h"
#include "uuid.h"
#include "sys_lib.h"

PackArchive::PackArchive(ILogger &log)
    : m_log(log)
{

}

std::vector<std::string> PackArchive::GetImages()
{
    std::vector<std::string> imgList;

    for (uint32_t i = 0; i < ni_get_number_of_images(); i++)
    {
        char buffer[13];
        ni_get_image(buffer, i);
        imgList.push_back(buffer);
    }

    return imgList;
}


void PackArchive::Unzip(const std::string &filePath, const std::string &parent_dest_dir)
{
    // std::string fileName = GetFileName(filePath);
    // std::string ext = GetFileExtension(fileName);
    // EraseString(fileName, "." + ext); // on retire l'extension du pack

    // std::string path = parent_dest_dir  + "/" + ToUpper(fileName);


    if (std::filesystem::exists(parent_dest_dir))
    {
        // 1. First delete files
        for(const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(parent_dest_dir))
        {
            if (std::filesystem::is_regular_file(entry)) {
                std::filesystem::remove(entry.path());
            }
        }
    }
    // 2. then delete directories
    std::filesystem::remove_all(parent_dest_dir);
    std::filesystem::create_directories(parent_dest_dir);


    (void) Zip::Unzip(filePath, parent_dest_dir, "");
}

static bool endsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

bool StringToFile(const std::string &filePath, const std::string &data)
{
    bool success = false;
    std::ofstream outFile(filePath, std::ifstream::out);

    if (outFile.is_open())
    {
        outFile << data << std::endl;
        outFile.close();
        success = true;
    }
    return success;
}

void PackArchive::DecipherFileOnDisk(const std::string &fileName)
{
    FILE * pFile;
    long lSize = 512;
    char * buffer;
    size_t result;

    pFile = fopen ( fileName.c_str() , "rb+" );
    if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

    // allocate memory to contain the whole file:
    buffer = (char*) malloc (sizeof(char)*lSize);
    if (buffer == NULL) {fprintf(stderr, "Memory error: %s", fileName.c_str()); exit (2);}

    // copy the file into the buffer:
    result = fread (buffer,1,lSize,pFile);
    if (result != lSize) {fprintf(stderr, "Reading error: %s", fileName.c_str()); exit (3);}

    /* the whole file is now loaded in the memory buffer. */

    // Decipher
    ni_decode_block512(reinterpret_cast<uint8_t *>(buffer));
    fseek (pFile , 0 , SEEK_SET); // ensure we are at the begining

    // Write back data on the disk
    fwrite (buffer , sizeof(char), lSize, pFile);

    // terminate
    fclose (pFile);
    free (buffer);
}

void WriteDataOnDisk(const std::string &fileName, const uint8_t *data, uint32_t size)
{
    FILE * pFile;
    pFile = fopen ( fileName.c_str() , "wb" );
    fwrite (data , sizeof(char), size, pFile);
    fclose (pFile);
}

void PackArchive::DecipherFiles(const std::string &directory, const std::string &suffix)
{
    for (const auto & rf : std::filesystem::directory_iterator(directory))
    {
        std::string oldFile = rf.path().generic_string();
//        std::cout << oldFile << std::endl;

        DecipherFileOnDisk(oldFile);

        std::string newName = oldFile + suffix;
        rename(oldFile.c_str(), newName.c_str());
    }
}

std::vector<std::string> PackArchive::FilesInMemory(const std::string &type, const uint8_t *data, uint32_t nb_elements)
{
    char res_file[13]; // 12 + \0
    std::vector<std::string> resList;

    for (int i = 0; i < nb_elements; i++)
    {
        memcpy(res_file, &data[i*12], 12);
        res_file[12] = '\0';

        std::string res_file_string(res_file);
        SysLib::ReplaceCharacter(res_file_string, "\\", "/");

        resList.push_back(res_file_string);
    }
    return resList;
}

void PackArchive::ConvertCommercialFormat(StoryProject &proj, const std::string &basePath)
{
    ResourceManager res(m_log);

    // RI file is not ciphered
    uint8_t data[512];
    uint32_t size = ni_get_ri_block(data);

    auto page = proj.GetPage(proj.MainUuid());

    // Images resources
    {
       std::vector<std::string> lst = FilesInMemory("ri", data, mNiFile.image_assets_count);

        for (auto &l : lst)
        {
            // Le path est de la forme "000/AE123245" où 000 est un répertoire

            auto rData = std::make_shared<Resource>();

            // origin
            auto from =  std::filesystem::path(basePath) / mPackName / "rf" / l;
            from += ".bmp";

            // destination
            auto filename =  SysLib::GetFileName(l) + ".bmp";
            auto to = proj.AssetsPath() / filename;

            rData->file =  filename;
            rData->type = ResourceManager::ExtentionInfo(rData->file, ResourceManager::InfoType);
            rData->format = ResourceManager::ExtentionInfo(rData->file, ResourceManager::InfoFormat);
            res.Add(rData);

            std::filesystem::copy(from, to, std::filesystem::copy_options::overwrite_existing);
        }
    }

    size = ni_get_si_block(data);

    // Sound files
    {
        std::vector<std::string> lst = FilesInMemory("si", data, mNiFile.sound_assets_count);

        for (auto &l : lst)
        {
            auto rData = std::make_shared<Resource>();

            // origin
            auto from =  std::filesystem::path(basePath) / mPackName / "sf" / l;
            from += ".mp3";

            // destination
            auto filename =  SysLib::GetFileName(l) + ".mp3";
            auto to = proj.AssetsPath() / filename;

            rData->file =  filename;
            rData->type = ResourceManager::ExtentionInfo(rData->file, ResourceManager::InfoType);
            rData->format = ResourceManager::ExtentionInfo(rData->file, ResourceManager::InfoFormat);
            res.Add(rData);

            std::filesystem::copy(from, to, std::filesystem::copy_options::overwrite_existing);
        }
    }

    size = ni_get_li_block(data);

    std::vector<uint32_t> transitions;
    // each entry of the transitions array is a 32-bit integer
    for (int i = 0; i < size;)
    {
        transitions.push_back(leu32_get(&data[i]));
        i += 4;
    }

    node_info_t node_info;

    // key: node index, value: node uuidV4
    std::map<int, std::string> nodeIds;

    //  key: node index, value: list of transitions
    std::map<int, std::vector<uint32_t>> nodeTransitions;

    for (int i = 0; i < mNiFile.node_size; i++)
    {
        ni_get_node_info(i, &node_info);

        auto node = proj.CreateNode(proj.MainUuid(), "media-node");

        if (node)
        {
            // On sauvegarde la relation entre l'index du noeud et son UUID
            nodeIds[i] = node->GetId();

            node->SetPosition(80 * i, 80 * i);
        
            nlohmann::json internalData;
            auto img = SysLib::GetFileName(node_info.ri_file);
            auto snd = SysLib::GetFileName(node_info.si_file);
            internalData["image"] = img.size() > 0 ? img + ".bmp" : "";
            internalData["sound"] = snd.size() > 0 ? snd + ".mp3" : "";

            node->SetInternalData(internalData);

            std::vector<uint32_t> jumpArray;

            // Autre cas
            if (node_info.current->ok_transition_number_of_options == 10)
            {
                // For now, consider that this is a bad format
                // In the future, consider using ok_transition_selected_option_index when ok_transition_number_of_options == 10

                // 00 00 00 00   ==> ok transition à zéro
                // 0A 00 00 00   ==> nombre spécial, ou vraiment l'offset dans le fichier LI ?
                // 01 00 00 00   ==> l'index dans le fichier LI à l'offset (disons, le premier élément)


                jumpArray.push_back(transitions[node_info.current->ok_transition_action_node_index_in_li]);
            }
            else
            {
                // Vraies transitions
                 for (int jIndex = 0; jIndex < node_info.current->ok_transition_number_of_options; jIndex++)
                {
                    jumpArray.push_back(transitions[node_info.current->ok_transition_action_node_index_in_li + jIndex]);
                }
            }

            nodeTransitions[i] = jumpArray;
        }
        else
        {
            std::cout << "Node not created" << std::endl;
            m_log.Log("Node not created");
        }
    }

    // Create links, parse again the nodes
    for (int i = 0; i < mNiFile.node_size; i++)
    {

        for (auto &j : nodeTransitions[i])
        {
            auto c = std::make_shared<Connection>();

            c->outNodeId = nodeIds[i];
            c->outPortIndex = 0;
            c->inNodeId = nodeIds[j];
            c->inPortIndex = 0;

            page->AddLink(c);
        }
    }

    proj.Save(res);
}


bool PackArchive::LoadNiFile(const std::string &filePath)
{
    bool success = false;
    mZip.Close();
    mCurrentNodeId = 0;

    std::string fileName = SysLib::GetFileName(filePath);
    std::string ext = SysLib::GetFileExtension(fileName);
    SysLib::EraseString(fileName, "." + ext); // on retire l'extension du pack
    mPackName = SysLib::ToUpper(fileName);

    std::cout << "Pack name: " << mPackName << std::endl;

    if (mZip.Open(filePath, true))
    {
        std::cout << "Number of files: " << mZip.NumberOfEntries() << std::endl;

        if (ParseNIFile(mPackName))
        {
            success = true;
            std::cout << "Parse NI file success\r\n"  << std::endl;
            ni_dump(&mNiFile);

            ni_get_node_info(mCurrentNodeId, &mCurrentNode);
        }
        else
        {
            std::cout << "Parse NI file error\r\n"  << std::endl;
        }
    }
    return success;
}

std::string PackArchive::OpenImage(const std::string &fileName)
{
    std::string f;
    mZip.GetFile(fileName, f);
    return f;
}

bool PackArchive::ConvertJsonStudioToOst(const std::string &basePath, const std::string &uuid, const std::string &outputDir)
{
    try
    {

        // STUDIO format
        std::ifstream f(basePath + "/story.json");
        nlohmann::json j = nlohmann::json::parse(f);
        StoryProject proj(m_log);
        ResourceManager res(m_log);

        std::shared_ptr<StoryPage> page = proj.CreatePage(proj.MainUuid());

        if (j.contains("title"))
        {
            proj.New(uuid, outputDir);
            proj.SetName(j["title"].get<std::string>());

            // Create resources, scan asset files
            std::filesystem::path directoryPath(basePath + "/assets");
            if (std::filesystem::exists(directoryPath) && std::filesystem::is_directory(directoryPath))
            {
                for (const auto& entry : std::filesystem::directory_iterator(directoryPath))
                {
                    if (std::filesystem::is_regular_file(entry.path()))
                    {
                        auto rData = std::make_shared<Resource>();

                        rData->file =  entry.path().filename().generic_string();
                        rData->type = ResourceManager::ExtentionInfo(entry.path().extension().generic_string(), ResourceManager::InfoType);
                        rData->format = ResourceManager::ExtentionInfo(entry.path().extension().generic_string(), ResourceManager::InfoFormat);
                        res.Add(rData);
                    }
                }
            }

            // Key: actionNode, value: Stage UUID
            std::map<std::string, std::string> stageActionLink;


            for (const auto & n : j["stageNodes"])
            {
                auto node = proj.CreateNode(proj.MainUuid(), "media-node");

                if (node)
                {
                    auto node_uuid = n["uuid"].get<std::string>();
                    node->SetId(node_uuid);
                    node->SetPosition(n["position"]["x"].get<float>(), n["position"]["y"].get<float>());
                
                    nlohmann::json internalData;
                    auto img = n["image"];
                    internalData["image"] = img.is_string() ? img.get<std::string>() : "";
                    auto audio = n["audio"];
                    internalData["sound"] = audio.is_string() ? audio.get<std::string>() : "";

                    node->SetInternalData(internalData);

                    stageActionLink[n["okTransition"]["actionNode"]] = node_uuid;
                }

            }

            for (const auto & n : j["actionNodes"])
            {
                std::string action_node_uuid = n["id"].get<std::string>(); // le champs est "id" et non pas "uuid", pénible

                if (stageActionLink.count(action_node_uuid) > 0)
                {
                    int i = 0;
                    for (const auto & m : n["options"])
                    {
                        auto c = std::make_shared<Connection>();

                        c->outNodeId = stageActionLink[action_node_uuid];
                        c->outPortIndex = i;
                        c->inNodeId = m; // On prend le stage node; 
                        c->inPortIndex = 0;

                        i++;
                        page->AddLink(c);
                    }
                }
                else
                {
                    std::cout << "ActionNode UUID not found" << std::endl;
                }
            }
             // Save on disk
            proj.Save(res);
        }
    }
    catch(std::exception &e)
    {
        m_log.Log(std::string("Import failure: ") + e.what());
    }

    return true; // FIXME
}

bool PackArchive::ImportStudioFormat(const std::string &fileName, const std::string &outputDir)
{   
    auto uuid =  Uuid().String();
    std::string basePath = outputDir + "/" + uuid;
    Unzip(fileName, basePath);

    ConvertJsonStudioToOst(basePath, uuid, outputDir);
    return false;
}

std::string PackArchive::GetImage(const std::string &fileName)
{
    //"C8B39950DE174EAA8E852A07FC468267/rf/000/05FB5530"
    std::string imagePath = mPackName + "/rf/" + fileName;
    SysLib::ReplaceCharacter(imagePath, "\\", "/");

    std::cout << "Loading " + imagePath << std::endl;
    return OpenImage(imagePath);
}

std::string PackArchive::CurrentImage()
{
    return GetImage(std::string(mCurrentNode.ri_file));
}

std::string PackArchive::CurrentSound()
{
    //"C8B39950DE174EAA8E852A07FC468267/sf/000/05FB5530"
    std::string soundPath = mPackName + "/sf/" + std::string(mCurrentNode.si_file);
    SysLib::ReplaceCharacter(soundPath, "\\", "/");

    std::cout << "Loading " + soundPath << std::endl;

    std::string f;
    if (mZip.GetFile(soundPath, f))
    {
        ni_decode_block512(reinterpret_cast<uint8_t *>(f.data()));
        return f;
    }
    else
    {
        std::cout << "Cannot load file from ZIP" << std::endl;
    }
    return "";
}

std::string PackArchive::CurrentSoundName()
{
    return std::string(mCurrentNode.si_file);
}

bool PackArchive::AutoPlay()
{
    return mCurrentNode.current->auto_play;
}

bool PackArchive::IsRoot() const
{
    return mCurrentNodeId == 0;
}

bool PackArchive::IsWheelEnabled() const
{
    return mCurrentNode.current->wheel;
}

void PackArchive::Next()
{
    // L'index de circulation dans le tableau des transitions commence à 1 (pas à zéro ...)
    uint32_t index = 1;
    if (mCurrentNode.current->ok_transition_selected_option_index < mNodeForChoice.current->ok_transition_number_of_options)
    {
        index = mCurrentNode.current->ok_transition_selected_option_index + 1;
    }
    // sinon on revient à l'index 0 (début du tableau des transitions)

    mCurrentNodeId = ni_get_node_index_in_li(mNodeForChoice.current->ok_transition_action_node_index_in_li, index - 1);
    ni_get_node_info(mCurrentNodeId, &mCurrentNode);
}

void PackArchive::Previous()
{
    // L'index de circulation dans le tableau des transitions commence à 1 (pas à zéro ...)
    uint32_t index = 1;
    if (mCurrentNode.current->ok_transition_selected_option_index > 1)
    {
        index = mCurrentNode.current->ok_transition_selected_option_index - 1;
    }
    else
    {
        index = mNodeForChoice.current->ok_transition_number_of_options;
    }

    mCurrentNodeId = ni_get_node_index_in_li(mNodeForChoice.current->ok_transition_action_node_index_in_li, index - 1);
    ni_get_node_info(mCurrentNodeId, &mCurrentNode);
}

void PackArchive::OkButton()
{
    if (mCurrentNode.current->home_transition_number_of_options > 0)
    {
        // On doit faire un choix!
        // On sauvegarde ce noeud car il va servir pour naviguer dans les choix
        mNodeIdForChoice = mCurrentNodeId;
        ni_get_node_info(mNodeIdForChoice, &mNodeForChoice);
    }
    mCurrentNodeId = ni_get_node_index_in_li(mCurrentNode.current->ok_transition_action_node_index_in_li, mCurrentNode.current->ok_transition_selected_option_index);
    ni_get_node_info(mCurrentNodeId, &mCurrentNode);
}

bool PackArchive::HasImage()
{
    return std::string(mCurrentNode.ri_file).size() > 1;
}

bool PackArchive::ParseNIFile(const std::string &root)
{
    bool success = true;
    std::string f;
    if (mZip.GetFile(root + "/li", f))
    {
        ni_set_li_block(reinterpret_cast<const uint8_t *>(f.data()), f.size());
    }
    else
    {
        success = false;
        std::cout << "[PACK_ARCHIVE] Cannot find LI file" << std::endl;
    }

    if (mZip.GetFile(root + "/ri", f))
    {
        ni_set_ri_block(reinterpret_cast<const uint8_t *>(f.data()), f.size());
    }
    else
    {
        success = false;
        std::cout << "[PACK_ARCHIVE] Cannot find RI file" << std::endl;
    }

    if (mZip.GetFile(root + "/si", f))
    {
        ni_set_si_block(reinterpret_cast<const uint8_t *>(f.data()), f.size());
    }
    else
    {
        success = false;
        std::cout << "[PACK_ARCHIVE] Cannot find SI file" << std::endl;
    }

    if (mZip.GetFile(root + "/ni", f))
    {
        success = success & ni_parser(&mNiFile, reinterpret_cast<const uint8_t *>(f.data()));
    }
    else
    {
        std::cout << "[PACK_ARCHIVE] Cannot find NI file" << std::endl;
    }
    return success;
}

std::string PackArchive::HexDump(const char *desc, const void *addr, int len)
{
    int i;
    unsigned char buff[17];
    const unsigned char *pc = static_cast<const unsigned char*>(addr);
    std::stringstream ss;

    // Output description if given.
    if (desc != nullptr)
    {
        ss << desc << ":\n";
    }

    if (len == 0) {
        ss << "  ZERO LENGTH\n";
        return ss.str();
    }
    if (len < 0) {
        ss << "  NEGATIVE LENGTH: " << len << "\n";
        return ss.str();
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                ss <<  "  " << buff << "\n";

            // Output the offset.
            ss <<  " " << std::setfill('0') << std::setw(4) << std::hex  << i;
        }

        // Now the hex byte_to_hexcode for the specific character.
        ss <<  " " << std::setfill('0') << std::setw(2) << std::hex  << static_cast<int>(pc[i]) << ", ";

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        ss <<  "   ";
        i++;
    }

    // And print the final ASCII bit.
    ss << "  "<< buff << "\n";
    return ss.str();
}

