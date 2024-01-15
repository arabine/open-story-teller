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

    void Save();
    void Scan();
private:
    std::string m_library_path;
    std::vector<std::shared_ptr<StoryProject>> m_projectsList;
};

#endif // LIBRARYMANAGER_H
