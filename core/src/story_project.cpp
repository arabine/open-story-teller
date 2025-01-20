

#include <fstream>
#include <iostream>
#include <filesystem>
#include <regex>

#include "story_project.h"
#include "json.hpp"
#include "media_node.h"
#include "function_node.h"
#include "sys_lib.h"

StoryProject::StoryProject(ILogger &log)
    : m_log(log)
{
    registerNode<MediaNode>("media-node");
    registerNode<FunctionNode>("function-node");
}

StoryProject::~StoryProject()
{
}

void StoryProject::SetPaths(const std::string &uuid, const std::string &library_path)
{
    m_uuid = uuid;
    m_project_file_path = std::filesystem::path(library_path) / uuid / std::filesystem::path("project.json");

    m_working_dir = m_project_file_path.parent_path().generic_string();
    m_assetsPath = m_working_dir /  std::filesystem::path("assets");

    std::cout << "Working dir is: " << m_working_dir << std::endl;
}

void StoryProject::CopyToDevice(const std::string &outputDir)
{
    ResourceManager manager(m_log);
    
    Load(manager);  

    // Output dir is the root. Build an assets directory to the device location
    std::filesystem::path destRootDir = std::filesystem::path(outputDir) / m_uuid;
    std::filesystem::path destAssetsDir = destRootDir /  "assets";
    std::filesystem::create_directories(destAssetsDir);

    // Generate and copy binary
    std::string code;
    GenerateScript(code);

    std::cout << code << std::endl;

    Chip32::Assembler::Error err;
    if (GenerateBinary(code, err))
    {
        std::filesystem::copy(BinaryFileName(), destRootDir, std::filesystem::copy_options::overwrite_existing);

        // Convert resources (if necessary) and copy them to destination assets
        manager.ConvertResources(AssetsPath(), destAssetsDir, m_storyOptions.image_format, m_storyOptions.sound_format);
    }
}

void StoryProject::New(const std::string &uuid, const std::string &library_path)
{
    SetPaths(uuid, library_path);

    // First try to create the working directory
    if (!std::filesystem::is_directory(m_working_dir))
    {
        std::filesystem::create_directories(m_working_dir);
    }

    std::filesystem::create_directories(m_assetsPath);

    CreatePage(MainUuid());

    m_initialized = true;
}

std::filesystem::path StoryProject::BinaryFileName() const
{
    return m_working_dir / "story.c32";
}


void StoryProject::SaveBinary()
{
    std::ofstream o(BinaryFileName() , std::ios::out | std::ios::binary);
    o.write(reinterpret_cast<const char*>(m_program.data()), m_program.size());
    o.close();
}

bool StoryProject::ParseStoryInformation(nlohmann::json &j)
{
    bool success = false;

    if (j.contains("project"))
    {
        nlohmann::json projectData = j["project"];

        m_name = projectData["name"].get<std::string>();
        m_uuid = projectData["uuid"].get<std::string>();
        m_titleImage = projectData.value("title_image", "");
        m_titleSound = projectData.value("title_sound", "");

        success = true;
    }

    return success;
}

void StoryProject::ModelToJson(nlohmann::json &model)
{
    for (const auto & p : m_pages)
    {
        nlohmann::json page = p->ToJson();
        model.push_back(page);
    }
}

std::shared_ptr<StoryPage> StoryProject::CreatePage(const std::string &uuid)
{
    auto newPage = std::make_shared<StoryPage>(uuid);
    m_pages.push_back(newPage);
    return newPage;
}

std::shared_ptr<BaseNode> StoryProject::CreateNode(const std::string_view &page_uuid, const std::string &type)
{
    typename Registry::const_iterator i = m_registry.find(type);
    if (i == m_registry.end()) {
        throw std::invalid_argument(std::string(__PRETTY_FUNCTION__) +
                                    ": key not registered");
    }
    else
    {
        auto n = i->second(type);

        for (auto & p : m_pages)
        {
            if (p->Uuid() == page_uuid)
            {
                p->AddNode(n);
            }
        }
        return n;
    }
}

void StoryProject::AddConnection(const std::string_view &page_uuid, std::shared_ptr<Connection> c)
{
    for (const auto & p : m_pages)
    {
        if (p->Uuid() == page_uuid)
        {
            p->AddLink(c);
        }
    }
}

void StoryProject::DeleteNode(const std::string_view &page_uuid, const std::string &id)
{
    for (const auto & p : m_pages)
    {
        if (p->Uuid() == page_uuid)
        {
            p->DeleteNode(id);
        }
    }
}

std::shared_ptr<StoryPage> StoryProject::GetPage(const std::string &uuid)
{
    for (const auto & p : m_pages)
    {
        if (p->Uuid() == uuid)
        {
            return p;
        }
    }

    return nullptr;

}

void StoryProject::DeleteLink(const std::string_view &page_uuid, std::shared_ptr<Connection> c)
{

    for (const auto & p : m_pages)
    {
        if (p->Uuid() == page_uuid)
        {
            p->DeleteLink(c);
        }
    }
}

std::pair<std::list<std::shared_ptr<BaseNode>>::iterator, std::list<std::shared_ptr<BaseNode>>::iterator> StoryProject::Nodes(const std::string_view &page_uuid)
{
    for (const auto & p : m_pages)
    {
        if (p->Uuid() == page_uuid)
        {
            return p->Nodes();
        }
    }

    return std::pair<std::list<std::shared_ptr<BaseNode>>::iterator, std::list<std::shared_ptr<BaseNode>>::iterator>();
}

std::pair<std::list<std::shared_ptr<Connection>>::iterator, std::list<std::shared_ptr<Connection>>::iterator> StoryProject::Links(const std::string_view &page_uuid)
{
    for (const auto & p : m_pages)
    {
        if (p->Uuid() == page_uuid)
        {
            return p->Links();
        }
    }

    return std::pair<std::list<std::shared_ptr<Connection>>::iterator, std::list<std::shared_ptr<Connection>>::iterator>();
}

bool StoryProject::ModelFromJson(const nlohmann::json &model)
{
    bool success = false;
    try {

        nlohmann::json pagesJsonArray = model["pages"];
        m_pages.clear();

        for (auto& pageModel : pagesJsonArray)
        {
            // 1. Create the page in memory
            auto p = std::make_shared<StoryPage>(pageModel["uuid"].get<std::string>());
            m_pages.push_back(p);

            // 2. Load the nodes
            nlohmann::json nodesJsonArray = pageModel["nodes"];
            for (auto& element : nodesJsonArray) {
            
                std::string type = element["type"].get<std::string>();

                auto n = CreateNode(p->Uuid(), type);
                if (n)
                {
                    n->FromJson(element);
                    n->Initialize();
                }
                else
                {
                    throw std::logic_error(std::string("No registered model with name ") + type);
                }
            }

            // 3. Load the connections
            // std::cout << model.dump(4) << std::endl;

            // Ici on reste flexible sur les connexions, cela permet de créer éventuellement des 
            // projets sans fils (bon, l'élément devrait quand même exister dans le JSON)
            if (pageModel.contains("connections"))
            {
                nlohmann::json connectionJsonArray = pageModel["connections"];

                // key: node UUID, value: output counts
                std::map<std::string, int> outputCounts;

                for (auto& connection : connectionJsonArray)
                {
                    p->AddLink(std::make_shared<Connection>(connection.get<Connection>()));
                }
            }


        }

       
        success = true;
    }
    catch(nlohmann::json::exception &e)
    {
        std::cout << "(NodeEditorWindow::Load) " << e.what() << std::endl;
    }

    return success;
}

bool StoryProject::CopyProgramTo(uint8_t *memory, uint32_t size)
{
    bool success = false;
    // Update ROM memory
    if (m_program.size() < size)
    {
        std::copy(m_program.begin(), m_program.end(), memory);
        success = true;
    }
    return success;
}

bool StoryProject::GetAssemblyLine(uint32_t pointer_counter, uint32_t &line)
{
    bool success = false;
    // On recherche quelle est la ligne qui possède une instruction à cette adresse
    std::vector<Chip32::Instr>::const_iterator ptr = m_assembler.Begin();
    for (; ptr != m_assembler.End(); ++ptr)
    {
        if ((ptr->addr == pointer_counter) && ptr->isRomCode())
        {
            break;
        }
    }

    if (ptr != m_assembler.End())
    {
        line = ptr->line;
        success = true; 
    }

    return success;
}

std::list<std::shared_ptr<Connection>> StoryProject::GetNodeConnections(const std::string &nodeId)
{
    std::list<std::shared_ptr<Connection>> c;

    for (const auto &p : m_pages)
    {
        p->GetNodeConnections(c, nodeId);
    }

    return c;
}

int StoryProject::OutputsCount(const std::string &nodeId)
{
    for (const auto &p : m_pages)
    {
        return p->OutputsCount(nodeId);
    }
    return 0;
}

bool StoryProject::UseResource(const std::string &label)
{
    bool used = m_usedLabels.contains(label);

    if (!used)
    {
        m_usedLabels.insert(label);
    }
    return used;
}

bool StoryProject::GenerateScript(std::string &codeStr)
{
    std::stringstream code;
    std::stringstream chip32;
    std::string firstNode;

    for (const auto & p : m_pages)
    {
        firstNode = p->FindFirstNode();
    }

    if (firstNode == "")
    {
        m_log.Log("First node not found, there must be only one node with a free input.");
        return false;
    }

    code << "\tjump    " << BaseNode::GetEntryLabel(firstNode) << "\r\n";
    
    // Empty resources usage
    m_usedLabels.clear();

    // On build toutes les pages
    for (const auto & p : m_pages)
    {
        p->Build(code, *this);
    }

    codeStr = code.str();

    // Add our utility functions
    std::string buffer;

    std::ifstream f("scripts/media.chip32");
    f.seekg(0, std::ios::end);
    buffer.resize(f.tellg());
    f.seekg(0);
    f.read(buffer.data(), buffer.size());
    codeStr += buffer;

    return true;
}

bool StoryProject::GenerateBinary(const std::string &code, Chip32::Assembler::Error &err)
{
    Chip32::Result result;
    bool success = false;

    if (m_assembler.Parse(code) == true)
    {
        if (m_assembler.BuildBinary(m_program, result) == true)
        {
            result.Print();

            m_log.Log("Binary successfully generated.");
            SaveBinary();
            success = true;
        }
        else
        {
            err = m_assembler.GetLastError();
            
        }
    }
    else
    {
        err = m_assembler.GetLastError();
        m_log.Log(err.ToString(), true);
    }
    return success;
}



bool StoryProject::Load(ResourceManager &manager)
{
    try {
        std::ifstream f(m_project_file_path);
        nlohmann::json j = nlohmann::json::parse(f);

        manager.Clear();

        if (ParseStoryInformation(j))
        {
            if (j.contains("resources"))
            {
                nlohmann::json resourcesData = j["resources"];

                for (const auto &obj : resourcesData)
                {
                    auto rData = std::make_shared<Resource>();

                    rData->type = obj["type"].get<std::string>();
                    rData->format = obj["format"].get<std::string>();
                    rData->description = obj["description"].get<std::string>();
                    rData->file = obj["file"].get<std::string>();
                    manager.Add(rData);
                }

                if (j.contains("pages"))
                {
                    ModelFromJson(j);
                    m_initialized = true;
                }
            }
        }
    }
    catch(nlohmann::json::exception &e)
    {
        std::cout << e.what() << std::endl;
    }

    if (m_pages.size() == 0)
    {
        CreatePage(MainUuid());
    }

    return m_initialized;
}

void StoryProject::Save(ResourceManager &manager)
{
    nlohmann::json j;
    j["project"] = { {"name", m_name}, {"uuid", m_uuid}, { "title_image", m_titleImage }, { "title_sound", m_titleSound } };

    {
        nlohmann::json resourcesData;

        auto [b, e] = manager.Items();
        for (auto it = b; it != e; ++it)
        {
            nlohmann::json obj = {{"type", (*it)->type},
                                  {"format", (*it)->format},
                                  {"description", (*it)->description},
                                  {"file", (*it)->file}};

            resourcesData.push_back(obj);
        }
        j["resources"] = resourcesData;
    }

    nlohmann::json model;
    ModelToJson(model);
    j["pages"] = model;

    std::ofstream o(m_project_file_path);
    o << std::setw(4) << j << std::endl;
}


void StoryProject::Clear()
{
    m_uuid = "";
    m_working_dir = "";
    m_project_file_path = "";
    m_initialized = false;
}


void StoryProject::SetTitleImage(const std::string &titleImage)
{
    m_titleImage = titleImage;
}

void StoryProject::SetTitleSound(const std::string &titleSound)
{
    m_titleSound = titleSound;
}

void StoryProject::SetImageFormat(Resource::ImageFormat format)
{
    m_storyOptions.image_format = format;
}

void StoryProject::SetSoundFormat(Resource::SoundFormat format)
{
    m_storyOptions.sound_format = format;
}

void StoryProject::SetDisplayFormat(int w, int h)
{
    m_storyOptions.display_w = w;
    m_storyOptions.display_h = h;
}

std::string StoryProject::GetProjectFilePath() const
{
    return m_project_file_path.generic_string();
}

std::string StoryProject::GetWorkingDir() const
{
    return m_working_dir.string();
}

std::string StoryProject::BuildFullAssetsPath(const std::string_view fileName) const
{
    return (AssetsPath() / fileName).generic_string();
}




