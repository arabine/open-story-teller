#include "story_project.h"

#include <fstream>
#include <iostream>
#include <queue>
#include <filesystem>
#include "json.hpp"

void StoryProject::New(const std::string &uuid, const std::string &file_path)
{
    m_uuid = uuid;
    Initialize(file_path);
}


void StoryProject::Initialize(const std::string &file_path)
{
    m_project_path = file_path;
    std::filesystem::path p(file_path);
    m_working_dir= p.parent_path();

    // Frist try to create the working directory
    if (!std::filesystem::is_directory(m_working_dir))
    {
        std::filesystem::create_directories(m_working_dir);
    }
    m_imagesPath = std::filesystem::path(m_working_dir) /  "images";
    m_soundsPath = std::filesystem::path(m_working_dir) /  "sounds";

    std::filesystem::create_directories(m_imagesPath);
    std::filesystem::create_directories(m_soundsPath);

    m_initialized = true;
}

bool StoryProject::Load(const std::string &file_path)
{

    std::ifstream f(file_path);
    bool success = false;
/*
    std::filesystem::path p(file_path);
    m_working_dir= p.parent_path();

    std::cout << "Working dir is: " << m_working_dir << std::endl;

    try {

        nlohmann::json j = nlohmann::json::parse(f);

        m_nodes.clear();
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


    } catch(std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }


    if (success)
    {
        CreateTree();
    }
*/
    return success;
}

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

        for (int i = 0; i < p->jumps.size(); i++)
        {
            int jump = p->jumps[i];

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

void StoryProject::AppendResource(const Resource &res)
{
    m_resources.push_back(res);
}

bool StoryProject::GetResourceAt(int index, Resource &resOut)
{
    bool success = false;
    if ((index >= 0) && (index < m_resources.size()))
    {
        resOut = m_resources[index];
        success = true;
    }
    return success;
}

void StoryProject::ClearResources()
{
    m_resources.clear();
}

std::string StoryProject::GetFileExtension(const std::string &fileName)
{
    if(fileName.find_last_of(".") != std::string::npos)
        return fileName.substr(fileName.find_last_of(".")+1);
    return "";
}

std::string StoryProject::Compile()
{
    std::stringstream chip32;

    chip32 << "\tjump         .entry\r\n";

    for (auto & r : m_resources)
    {
        std::string fileName = GetFileName(r.file);
        std::string ext = GetFileExtension(fileName);
        EraseString(fileName, "." + ext); // on retire l'extension du pack

        chip32 << "$" << fileName << " DC8 \"" << fileName + "." + ext << "\", 8\r\n";
    }
    chip32 << ".entry:\r\n";

    chip32 << "\thalt\r\n";

    return chip32.str();
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
    return m_project_path;
}

std::string StoryProject::GetWorkingDir() const
{
    return m_working_dir;
}

