#include "story_project.h"

#include <fstream>
#include <iostream>
#include <filesystem>

#include "json.hpp"

StoryProject::StoryProject()
{

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


bool StoryProject::Load(nlohmann::json &model, ResourceManager &manager)
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
                    model = j["nodegraph"];
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

void StoryProject::Save(const nlohmann::json &model, ResourceManager &manager)
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



