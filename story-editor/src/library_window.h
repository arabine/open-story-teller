#pragma once

#include "window_base.h"
#include "library_manager.h"
#include "i_story_manager.h"

struct StoryInf {
    int age;
    std::string title;
};

class LibraryWindow : public WindowBase
{
public:
    LibraryWindow(IStoryManager &project, LibraryManager &library);

    void Initialize();
    virtual void Draw() override;

private:
    IStoryManager &m_storyManager;
    LibraryManager &m_libraryManager;

    std::vector<StoryInf> m_store;

    std::string m_storeRawJson;
    void ParseStoreData();
};

