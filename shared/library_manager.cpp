#include "library_manager.h"
#include "tlv.h"
#include <filesystem>
#include <thread>

#include "json.hpp"
#include "story_project.h"
#include "uuid.h"


LibraryManager::LibraryManager(ILogger &log)
    : m_log(log)
{

}

LibraryManager::~LibraryManager()
{
    if (m_copyWorker.joinable())
    {
        m_copyWorker.join();
    }
}

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
                        auto proj = std::make_shared<StoryProject>(m_log);
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
                        catch(nlohmann::json::exception &e)
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
    auto story = std::make_shared<StoryProject>(m_log);
    std::string uuid = Uuid().String();

    story->New(uuid, m_library_path);
    story->SetDisplayFormat(320, 240);
    story->SetImageFormat(Resource::IMG_FORMAT_QOIF);
    story->SetSoundFormat(Resource::SND_FORMAT_WAV);
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

std::string LibraryManager::IndexFileName() const
{
    auto p = std::filesystem::path(m_library_path) / "index.ost";
    return p.string();
}

void LibraryManager::Save()
{
    Tlv tlv;

    tlv.add_object(2); // 2 éléments dans l'objet

    tlv.add_integer(1); // version du fichier (1 == V1)

    // On compte le nombre de projets sélecttionnés
    uint16_t stories = 0;
    for (auto &p : m_projectsList)
    {
        if (p->IsSelected())
        {
            stories++;
        }
    }

    tlv.add_array(stories); // Nombre d'histoires
    
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

    tlv.Save(IndexFileName());
}


void LibraryManager::CopyToDevice(const std::string &outputDir)
{
    try
    {  
        // Generate TLV file (index of all stories)
        Save();
        // Copy TLV to the directory root
        std::filesystem::copy(IndexFileName(), outputDir, std::filesystem::copy_options::overwrite_existing);

        if (m_copyWorker.joinable())
        {
            m_copyWorker.join();
        }

        m_copyWorker = std::thread([&, outputDir]() {
         //   myThread.detach();

            std::cout << "Starting to copy elements" << std::endl;

            for (auto p : *this)
            {
                if (p->IsSelected())
                {
                    std::cout << "Copying " << p->GetName() << std::endl;
                    p->CopyToDevice(outputDir);
                }
            }
        }); 
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    } catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

bool LibraryManager::IsInitialized() const
{
    return m_library_path.size() > 0;
}

std::string LibraryManager::GetVersion()
{
   return std::to_string(VERSION_MAJOR) + '.' + std::to_string(VERSION_MINOR) + '.' + std::to_string(VERSION_PATCH);
}

void LibraryManager::AddStory(IStoryDb::Info &info, int origin)
{
    m_storyDb.AddStory(info, origin);
}

void LibraryManager::ParseCommunityStore(const std::string &jsonFileName)
{
    try {
        std::ifstream f(jsonFileName);
        nlohmann::json j = nlohmann::json::parse(f);

        if (!j.contains("data")) {
            throw std::runtime_error("Invalid JSON: 'data' key not found");
        }

        const auto& data = j["data"];

        m_storyDb.ClearCommunity();
        for (const auto &obj : data)
        {
            IStoryDb::Info s;

            s.title = obj["title"].get<std::string>();
            s.description = obj["description"].get<std::string>();
            s.download = obj["download"].get<std::string>();
            s.age = obj["age"].get<int>();

            m_storyDb.AddStory(s, StoryDb::cCommunityStore);
        }
    }
    catch(std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }
}

void LibraryManager::ParseCommercialStore(const std::string &jsonFileName)
{
    try
    {

        std::ifstream f(jsonFileName);
        nlohmann::json j = nlohmann::json::parse(f);

        if (!j.contains("response")) {
            throw std::runtime_error("Invalid JSON: 'response' key not found");
        }

        const auto& response = j["response"];
        m_storyDb.ClearCommercial();

        for (auto it = response.begin(); it != response.end(); ++it)
        {
            const auto& pack = it.value();

            IStoryDb::Info story;
            story.title = pack["title"].get<std::string>();
            story.uuid = pack["uuid"].get<std::string>();

            if (pack.contains("localized_infos") && pack["localized_infos"].contains("fr_FR"))
            {
                const auto& localized = pack["localized_infos"]["fr_FR"];
                story.title = localized["title"].get<std::string>();
                story.description = localized["description"].get<std::string>();

                if (localized.contains("image") && localized["image"].contains("image_url")) {
                    story.image_url = localized["image"]["image_url"].get<std::string>();
                }
            }

            m_storyDb.AddStory(story, StoryDb::cCommercialStore);
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
}

