#ifndef LIBRARYMANAGER_H
#define LIBRARYMANAGER_H

#include <string>
#include <vector>
#include <memory>
#include "story_project.h"

class LibraryManager
{
public:
    LibraryManager();

    void Initialize(const std::string &library_path);
    bool IsInitialized() const;

    std::string LibraryPath() const { return m_library_path; }
    static std::string GetVersion();

    std::vector<std::shared_ptr<StoryProject>>::const_iterator begin() const { return m_projectsList.begin(); }
    std::vector<std::shared_ptr<StoryProject>>::const_iterator end() const { return m_projectsList.end(); }

    uint32_t ProjectsCount() const { return m_projectsList.size(); };

    void Save();
    void Scan();

    // Copie toutes les histoires sélectionnées vers un répertoire
    // On va ne copier que les fichiers de sortie au format désiré 
    void CopyToDevice(const std::string &outputDir);

    std::shared_ptr<StoryProject> NewProject();
    void CheckDirectories();

    std::shared_ptr<StoryProject> GetStory(const std::string &uuid);

    void SetStoreUrl(const std::string &store_url) { m_storeUrl = store_url; }
    std::string GetStoreUrl() const { return m_storeUrl; }

private:
    std::string m_library_path;
    std::vector<std::shared_ptr<StoryProject>> m_projectsList;
    std::string m_storeUrl;
};

#endif // LIBRARYMANAGER_H
