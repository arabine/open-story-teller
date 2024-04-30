#include "library_manager.h"
#include "tlv.h"
#include <filesystem>
#include <thread>

#include "json.hpp"
#include "story_project.h"
#include "uuid.h"


LibraryManager::LibraryManager() {}

void LibraryManager::Initialize(const std::string &library_path)
{
    m_library_path = library_path;
    CheckDirectories();
    Scan();
}

void LibraryManager::CheckDirectories()
{
    std::filesystem::path dlDir = std::filesystem::path(m_library_path) / "store";
    std::filesystem::create_directories(dlDir);
}

void LibraryManager::Scan()
{
    std::filesystem::path directoryPath(m_library_path);
    if (std::filesystem::exists(directoryPath) && std::filesystem::is_directory(directoryPath))
    {
        m_projectsList.clear();
        for (const auto& entry : std::filesystem::directory_iterator(directoryPath))
        {
            if (std::filesystem::is_directory(entry.path()))
            {
                // Si c'est un sous-répertoire, récursivement scanner le contenu
                std::string uuid = entry.path().filename().generic_string();
                if (Uuid::IsValid(uuid))
                {
                    std::cout << "Found story directory" << std::endl;
                    // Look for a story.json file in this directory
                    auto p = entry.path() / "project.json";
                    if (std::filesystem::exists(p))
                    {
                        // okay, open it
                        auto proj = std::make_shared<StoryProject>();
                        try {
                            std::ifstream f(p);
                            nlohmann::json j = nlohmann::json::parse(f);

                            if (proj->ParseStoryInformation(j))
                            {
                                // Valid project file, add it to the list
                                proj->SetPaths(uuid, m_library_path);
                                m_projectsList.push_back(proj);
                            }
                        }
                        catch(std::exception &e)
                        {
                            std::cout << e.what() << std::endl;
                        }
                    }
                }
            }
        }
    }
}

std::shared_ptr<StoryProject> LibraryManager::NewProject()
{
    auto story = std::make_shared<StoryProject>();
    std::string uuid = Uuid().String();

    story->New(uuid, m_library_path);
    story->SetDisplayFormat(320, 240);
    story->SetImageFormat(StoryProject::IMG_FORMAT_QOIF);
    story->SetSoundFormat(StoryProject::SND_FORMAT_WAV);
    story->SetName("New project");
    return story;
}

std::shared_ptr<StoryProject> LibraryManager::GetStory(const std::string &uuid)
{
    std::shared_ptr<StoryProject> current;
    for (const auto &s : m_projectsList)
    {
        if (s->GetUuid() == uuid)
        {
            current = s;
        }
    }
    return current;
}

void LibraryManager::Save()
{
    auto p = std::filesystem::path(m_library_path) / "index.ost";
    Tlv tlv(p.string());

    tlv.add_object(1);
    tlv.add_string(GetVersion());

    tlv.add_array(m_projectsList.size());
    for (auto &p : m_projectsList)
    {
        if (p->IsSelected())
        {
            tlv.add_object(6);
            tlv.add_string(p->GetUuid());
            tlv.add_string(p->GetTitleImage());
            tlv.add_string(p->GetTitleSound());
            tlv.add_string(p->GetName());
            tlv.add_string(p->GetDescription());
            tlv.add_integer(p->GetVersion());
        }
    }

    /*


    // Title image
    std::string image =  RemoveFileExtension(m_titleImage) + ".qoi";
    tlv.add_string(image.c_str(), image.size());

    std::string sound =  RemoveFileExtension(m_titleSound) + ".wav";
    tlv.add_string(sound.c_str(), sound.size()); // title sound
*/
}


void LibraryManager::CopyToDevice(const std::string &outputDir)
{
    std::thread myThread([&]() {
        myThread.detach();

        std::cout << "Starting to copy elements" << std::endl;

        for (auto p : *this)
        {
            
        }
    }); 
}

bool LibraryManager::IsInitialized() const
{
    return m_library_path.size() > 0;
}

std::string LibraryManager::GetVersion()
{
   return std::to_string(VERSION_MAJOR) + '.' + std::to_string(VERSION_MINOR) + '.' + std::to_string(VERSION_PATCH);
}
