#pragma once

#include "window_base.h"
#include "library_manager.h"
#include "i_story_manager.h"

class LibraryWindow : public WindowBase
{
public:
    LibraryWindow(IStoryManager &project, LibraryManager &library);

    void Initialize();
    virtual void Draw() override;

private:
    IStoryManager &m_storyManager;
    LibraryManager &m_libraryManager;

};

