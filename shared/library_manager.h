#ifndef LIBRARYMANAGER_H
#define LIBRARYMANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <thread>

#include "story_project.h"
#include "i_logger.h"
#include "story_db.h"

class LibraryManager
{
public:
    LibraryManager(ILogger &log);
    ~LibraryManager();

    void Initialize(const std::string &library_path);
    bool IsInitialized() const;

    std::string LibraryPath() const { return m_library_path; }
    static std::string GetVersion();

    std::list<std::shared_ptr<StoryProject>>::const_iterator begin() const { return m_projectsList.begin(); }
    std::list<std::shared_ptr<StoryProject>>::const_iterator end() const { return m_projectsList.end(); }

    uint32_t ProjectsCount() const { return m_projectsList.size(); };

    void Save();
    void Scan();

    // Copie toutes les histoires sélectionnées vers un répertoire
    // On va ne copier que les fichiers de sortie au format désiré 
    void CopyToDevice(const std::string &outputDir);

    std::shared_ptr<StoryProject> NewProject();
    void CheckDirectories();

    std::shared_ptr<StoryProject> GetStory(const std::string &uuid);

    std::string IndexFileName() const;

    void SetStoreUrl(const std::string &store_url) { m_storeUrl = store_url; }
    std::string GetStoreUrl() const { return m_storeUrl; }

    void AddStory(IStoryDb::Info &info, int origin);

    void ParseCommunityStore(const std::string &jsonFileName);
    void ParseCommercialStore(const std::string &jsonFileName);

    auto CommunityDbView() const {
        return m_storyDb.CommunityDbView();
    }

    auto CommercialDbView() const {
        return m_storyDb.CommercialDbView();
    }

private:
    ILogger &m_log;
    std::string m_library_path;
    std::list<std::shared_ptr<StoryProject>> m_projectsList;
    std::string m_storeUrl;
    std::thread m_copyWorker;
    StoryDb m_storyDb;
};

#endif // LIBRARYMANAGER_H
