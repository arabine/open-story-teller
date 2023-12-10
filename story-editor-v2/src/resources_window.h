#pragma once

#include <vector>
#include <map>
#include <mutex>
#include "story_project.h"
#include "window_base.h"

class ResourcesWindow : public WindowBase
{
public:
    ResourcesWindow(StoryProject &project);
    ~ResourcesWindow();
    virtual void Draw() override;

private:
    StoryProject &m_project;

    bool m_showImportDialog{false};
    bool m_soundFile{false};

    void ChooseFile();
};
