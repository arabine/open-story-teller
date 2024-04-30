#include "story_project.h"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <regex>

#include "json.hpp"
#include "media_node.h"

StoryProject::StoryProject()
{
    registerNode<MediaNode>("media-node");
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
    ResourceManager manager;

    Load(manager);
    
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

    m_initialized = true;
}


void StoryProject::SaveBinary(const std::vector<uint8_t> &m_program)
{
    std::ofstream o(m_working_dir / "story.c32", std::ios::out | std::ios::binary);
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
    nlohmann::json nodes = nlohmann::json::array();
    for (const auto & n : m_nodes)
    {
        nodes.push_back(n->ToJson());
    }

    model["nodes"] = nodes;

    // Save links
    nlohmann::json connections = nlohmann::json::array();
    for (const auto& cnx : m_links)
    {
        nlohmann::json c(*cnx);
        connections.push_back(c);
        // Connection cnx = LinkToModel(linkInfo->ed_link->InputId, linkInfo->ed_link->OutputId);
    }

    model["connections"] = connections;
}


std::shared_ptr<BaseNode> StoryProject::CreateNode(const std::string &type)
{

    typename Registry::const_iterator i = m_registry.find(type);
    if (i == m_registry.end()) {
        throw std::invalid_argument(std::string(__PRETTY_FUNCTION__) +
                                    ": key not registered");
    }
    else
    {
        return i->second(type);
    }
}

void StoryProject::AddConnection(std::shared_ptr<Connection> c)
 {
    m_links.push_back(c);
}

void StoryProject::DeleteNode(const std::string &id)
{
    auto it = std::find_if(m_nodes.begin(), 
                       m_nodes.end(), 
                       [&id](std::shared_ptr<BaseNode> const &n) { return n->GetId() == id; });

    if ( it != m_nodes.end() )
    {
        it->reset();
        m_nodes.erase(it);
    }
}

void StoryProject::DeleteLink(std::shared_ptr<Connection> c)
{
    auto it = std::find_if(m_links.begin(), 
                       m_links.end(), 
                       [&c](std::shared_ptr<Connection> const &cnx) {
        return *cnx == *c; 
    });

    if ( it != m_links.end() )
    {
        it->reset();
        m_links.erase(it);
    }
}

bool StoryProject::ModelFromJson(const nlohmann::json &model)
{
    bool success = false;
    try {

        nlohmann::json nodesJsonArray = model["nodes"];

        m_nodes.clear();
        m_links.clear();

        for (auto& element : nodesJsonArray) {
           
            std::string type = element["type"].get<std::string>();

            auto n = CreateNode(type);
            if (n)
            {
                n->FromJson(element);
                m_nodes.push_back(n);
            }
            else
            {
                throw std::logic_error(std::string("No registered model with name ") + type);
            }
        }

        // std::cout << model.dump(4) << std::endl;

        // Ici on reste flexible sur les connexions, cela permet de créer éventuellement des 
        // projets sans fils (bon, l'élément devrait quand même exister dans le JSON)
        if (model.contains("connections"))
        {
            nlohmann::json connectionJsonArray = model["connections"];

            // key: node UUID, value: output counts
            std::map<std::string, int> outputCounts;

            for (auto& connection : connectionJsonArray)
            {
                m_links.push_back(std::make_shared<Connection>(connection.get<Connection>()));
            }
        }
        success = true;
    }
    catch(std::exception &e)
    {
        std::cout << "(NodeEditorWindow::Load) " << e.what() << std::endl;
    }
    return success;
}

int StoryProject::OutputsCount(const std::string &nodeId)
{
    int count = 0;
    for (const auto & l : m_links)
    { 
        if (l->outNodeId == nodeId)
        {
            count++;
        }
    }
    return count;
}

std::list<std::shared_ptr<Connection>> StoryProject::GetNodeConnections(const std::string &nodeId)
{
    std::list<std::shared_ptr<Connection>> c;

    for (const auto & l : m_links)
    { 
        if (l->outNodeId == nodeId)
        {
            c.push_back(l);
        }
    }

    return c;
}


std::string StoryProject::FindFirstNode() const
{
    std::string id;

    // First node is the one without connection on its input port

    for (const auto & n : m_nodes)
    {
        bool foundConnection = false;

        for (const auto& l : m_links)
        {
            if (l->inNodeId == n->GetId())
            {
                foundConnection = true;
            }
        }

        if (!foundConnection)
        {
            id = n->GetId();
            std::cout << "First node is: " + id << std::endl;
            break;
        }
    }

    return id;
}


bool StoryProject::Build(std::string &codeStr)
{
    std::stringstream code;
    std::stringstream chip32;

    std::string firstNode = FindFirstNode();

    if (firstNode == "")
    {
        std::cout << "First node not found, there must be only one node with a free input." << std::endl;
        return false;
    }

    code << "\tjump    " << BaseNode::GetEntryLabel(firstNode) << "\r\n";

    // First generate all constants
    for (const auto & n : m_nodes)
    {
        code << n->GenerateConstants(*this, OutputsCount(n->GetId())) << "\n";
    }

    for (const auto & n : m_nodes)
    {
        code << n->Build(*this, OutputsCount(n->GetId())) << "\n";
    }

    codeStr = code.str();
    return true;
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

                if (j.contains("nodegraph"))
                {
                    ModelFromJson(j["nodegraph"]);
                    m_initialized = true;
                }
            }
        }

 /*

        if (j.contains("nodes"))
        {
            for (auto& element : j["nodes"])
            {
                StoryNode n;

                n.auto_jump = element["auto_jump"];

                for (auto& jump : element["jumps"])
                {
                    n.jumps.push_back(jump.get<int>());
                }

                n.id = element["id"];
                n.image = element["image"];
                n.sound = element["sound"];

                m_nodes.push_back(n);
            }
        }

        m_images.clear();
        if (j.contains("images"))
        {
            for (auto& element : j["images"])
            {
                Resource r;

                r.file = element["file"];
                r.description = element["description"];
                r.format = element["format"];

                m_images.push_back(r);
            }
        }

        m_sounds.clear();
        if (j.contains("sounds"))
        {
            for (auto& element : j["sounds"])
            {
                Resource r;

                r.file = element["file"];
                r.description = element["description"];
                r.format = element["format"];

                m_sounds.push_back(r);
            }
        }

        m_type = j["type"];
        m_code = j["code"];
        m_name = j["name"];

        success = true;

*/
    }
    catch(std::exception &e)
    {
        std::cout << e.what() << std::endl;
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
    j["nodegraph"] = model;

    std::ofstream o(m_project_file_path);
    o << std::setw(4) << j << std::endl;
}
/*
void StoryProject::CreateTree()
{
    // Algorithm: level order traversal of N-ary tree
    std::queue<StoryNode *> nlist;

    m_tree = &m_nodes[0];
    nlist.push(m_tree);

    while (!nlist.empty())
    {
        StoryNode *p = nlist.front();
        std::cout << "Node: " << p->id << std::endl;

        for (size_t i = 0; i < p->jumps.size(); i++)
        {
            size_t jump = p->jumps[i];

            if (jump < m_nodes.size())
            {
                StoryNode *child = &m_nodes[jump];
                nlist.push(child);
                p->children.push_back(child);
            }
            else
            {
                std::cout << "End node" << std::endl;
            }
        }

        nlist.pop();
    }
}
*/

void StoryProject::Clear()
{
    m_uuid = "";
    m_working_dir = "";
    m_project_file_path = "";
    m_initialized = false;
    m_nodes.clear();
    m_links.clear();
}

void StoryProject::EraseString(std::string &theString, const std::string &toErase)
{
    std::size_t found;
    found = theString.find(toErase);
    if (found != std::string::npos)
    {
        theString.erase(found, toErase.size());
    }
}

std::string StoryProject::ToUpper(const std::string &input)
{
    std::string str = input;
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

std::string StoryProject::GetFileName(const std::string &path)
{
    auto found = path.find_last_of("/\\");
    return path.substr(found+1);
}


void StoryProject::ReplaceCharacter(std::string &theString, const std::string &toFind, const std::string &toReplace)
{
    std::size_t found;
    do
    {
        found = theString.find(toFind);
        if (found != std::string::npos)
        {
            theString.replace(found, 1, toReplace);
        }
    }
    while (found != std::string::npos);
}

std::string StoryProject::RemoveFileExtension(const std::string &FileName)
{
    std::string f = GetFileName(FileName);
    std::string ext = GetFileExtension(f);
    EraseString(f, "." + ext);
    return f;
}

std::string StoryProject::FileToConstant(const std::string &FileName, const std::string &extension)
{
    std::string f = RemoveFileExtension(FileName);
    return "$" + f + " DC8 \"" + f + extension + "\", 8\r\n";
}

std::string StoryProject::Normalize(const std::string &input)
{
    std::string valid_file = input;

    std::replace(valid_file.begin(), valid_file.end(), '\\', '_');
    std::replace(valid_file.begin(), valid_file.end(), '/', '_');
    std::replace(valid_file.begin(), valid_file.end(), ':', '_');
    std::replace(valid_file.begin(), valid_file.end(), '?', '_');
    std::replace(valid_file.begin(), valid_file.end(), '\"', '_');
    std::replace(valid_file.begin(), valid_file.end(), '<', '_');
    std::replace(valid_file.begin(), valid_file.end(), '>', '_');
    std::replace(valid_file.begin(), valid_file.end(), '|', '_');
    std::replace(valid_file.begin(), valid_file.end(), ' ', '_');

    return valid_file;
}

void StoryProject::SetTitleImage(const std::string &titleImage)
{
    m_titleImage = titleImage;
}

void StoryProject::SetTitleSound(const std::string &titleSound)
{
    m_titleSound = titleSound;
}

std::string StoryProject::GetFileExtension(const std::string &fileName)
{
    if(fileName.find_last_of(".") != std::string::npos)
        return fileName.substr(fileName.find_last_of(".")+1);
    return "";
}

void StoryProject::SetImageFormat(ImageFormat format)
{
    m_imageFormat = format;
}

void StoryProject::SetSoundFormat(SoundFormat format)
{
    m_soundFormat = format;
}

void StoryProject::SetDisplayFormat(int w, int h)
{
    m_display_w = w;
    m_display_h = h;
}

std::string StoryProject::GetProjectFilePath() const
{
    return m_project_file_path.generic_string();
}

std::string StoryProject::GetWorkingDir() const
{
    return m_working_dir.string();
}

std::string StoryProject::BuildFullAssetsPath(const std::string &fileName) const
{
    return (AssetsPath() / fileName).generic_string();
}



