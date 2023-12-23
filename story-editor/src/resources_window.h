#pragma once

#include <vector>
#include <map>
#include <mutex>
#include "i_story_manager.h"
#include "window_base.h"

class ResourcesWindow : public WindowBase
{
public:
    ResourcesWindow(IStoryManager &project);
    ~ResourcesWindow();
    virtual void Draw() override;

private:
    IStoryManager &m_story;

    bool m_showImportDialog{false};
    bool m_soundFile{false};

    void ChooseFile();
};
