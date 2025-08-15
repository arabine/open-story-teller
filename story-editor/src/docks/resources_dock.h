#pragma once

#include "i_story_manager.h"
#include "window_base.h"
#include "resource_manager.h"

class ResourcesDock : public WindowBase
{
public:
    ResourcesDock(ResourceManager &resources, IStoryManager &manager);
    ~ResourcesDock();
    virtual void Draw() override;

private:
    ResourceManager &m_resources;
    IStoryManager &m_storyManager;

    bool m_showImportDialog{false};
    bool m_soundFile{false};

    void ChooseFile();
};
