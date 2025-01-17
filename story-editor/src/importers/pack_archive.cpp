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

void PackArchive::ConvertCommercialFormat(StoryProject &proj, const std::filesystem::path &importBaseDir)
{
    ResourceManager res(m_log);

    // RI file is not ciphered
    uint8_t data[XI_BLOCK_SIZE];
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
            auto from =  importBaseDir / "rf" / l;
            from += ".bmp";

            // destination
            auto filename =  SysLib::GetFileName(l) + ".bmp";
            auto to = proj.AssetsPath() / filename;

            rData->file =  filename;
            rData->type = ResourceManager::ExtentionInfo(".bmp", ResourceManager::InfoType);
            rData->format = ResourceManager::ExtentionInfo(".bmp", ResourceManager::InfoFormat);
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
            auto from =  importBaseDir / "sf" / l;
            from += ".mp3";

            // destination
            auto filename =  SysLib::GetFileName(l) + ".mp3";
            auto to = proj.AssetsPath() / filename;

            rData->file =  filename;
            rData->type = ResourceManager::ExtentionInfo(".mp3", ResourceManager::InfoType);
            rData->format = ResourceManager::ExtentionInfo(".mp3", ResourceManager::InfoFormat);
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

    // key: index in transition table
    // value: list of node ids in desination
    std::map<int, std::vector<int>> referencedIndexes;

    // Direct nodes (no choices)
    // key: nodeID source, value: nodeID destination
    std::map<int, int>  nodeLinks;

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

         
            std::cout << i << "\t==> Node\t" << img << "\t" << snd << std::endl;
            std::cout << "\tOK node index in LI\t" << node_info.current->ok_btn_node_idx_in_li << std::endl;
            std::cout << "\tOK number of options\t" << node_info.current->ok_btn_size_or_base_idx << std::endl;
            std::cout << "\tOK selected option index\t" << node_info.current->ok_btn_offset_from_base << std::endl;

            node->SetInternalData(internalData);

            std::vector<uint32_t> jumpArray;
            



            if (node_info.current->wheel)
            {
                /*
                Exemple d'un noeud de choix (wheel == true) avec trois noeuds :
                node number                 13      17      21
                ok_btn_node_idx_in_li       36      36      36
                ok_btn_size_or_base_idx     18      18      18
                ok_btn_offset_from_base     0       2       4

                Dans ce cas: 
                    - le 18 est l'index de base où sont situés les 3 choix
                    - 36+0 est l'index où aller lors de l'appui sur OK pour le noeud 13
                    - 36+2 est l'index où aller lors de l'appui sur OK pour le noeud 17
                    - 36+4 est l'index où aller lors de l'appui sur OK pour le noeud 21
                
                */

                // On ajouter ce noeud à la liste des références
                // dans ce cas, le champs ok_btn_size_or_base_idx est interprété en tant que index
                referencedIndexes[node_info.current->ok_btn_size_or_base_idx].push_back(i);
                
                // On va créer le lien entre notre noeud et le noeud indiqué dans la table de transition
                nodeLinks[i] = transitions[node_info.current->ok_btn_size_or_base_idx + node_info.current->ok_btn_offset_from_base];
            }
            else
            {
                /*
                 ici, pas de wheel de sélection, donc c'est un son joué et un lien direct
                */   
                nodeLinks[i] = transitions[node_info.current->ok_btn_size_or_base_idx + node_info.current->ok_btn_offset_from_base];
            }

        }
        else
        {
            std::cout << "Node not created" << std::endl;
            m_log.Log("Node not created");
        }
    }

    // Create links, parse again the nodes
   // for (int i = 0; i < mNiFile.node_size; i++)
    //{
       //  std::cout << "Node id " << nodeIds[i] << " has " << nodeTransitions[i].size() << " transistions" << std::endl;

        for (auto &j : nodeLinks)
        {
            auto c = std::make_shared<Connection>();

            c->outNodeId = nodeIds[j.first]; // source
            c->outPortIndex = 0;
            c->inNodeId = nodeIds[j.second]; // destination
            c->inPortIndex = 0;

            page->AddLink(c);
        }
    //}

    proj.Save(res);
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

bool PackArchive::ParseRootFiles(const std::filesystem::path &root)
{
    bool success = true;
    std::string f;

    f = SysLib::ReadFile(root / "ri");
    if (f.size() > 0)
    {
        // Deciphering is done in this function
        ni_set_ri_block(reinterpret_cast<const uint8_t *>(f.data()), f.size());
    }
    else
    {
        success = false;
        std::cout << "[PACK_ARCHIVE] Cannot find RI file" << std::endl;
    }

    f = SysLib::ReadFile(root / "si");
    if (f.size() > 0)
    {
        // Deciphering is done in this function
        ni_set_si_block(reinterpret_cast<const uint8_t *>(f.data()), f.size());
    }
    else
    {
        success = false;
        std::cout << "[PACK_ARCHIVE] Cannot find SI file" << std::endl;
    }

    f = SysLib::ReadFile(root / "li");
    if (f.size() > 0)
    {
        // Deciphering is done in this function
        ni_set_li_block(reinterpret_cast<const uint8_t *>(f.data()), f.size());
    }
    else
    {
        success = false;
        std::cout << "[PACK_ARCHIVE] Cannot find LI file" << std::endl;
    }
    
    f = SysLib::ReadFile(root / "ni");
    if (f.size() > 0)
    {
        success = success & ni_parser(&mNiFile, reinterpret_cast<const uint8_t *>(f.data()));
    }
    else
    {
        success = false;
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

