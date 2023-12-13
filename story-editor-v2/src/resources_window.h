#pragma once

#include <vector>
#include <map>
#include <mutex>
#include "i_story_project.h"
#include "window_base.h"

class ResourcesWindow : public WindowBase
{
public:
    ResourcesWindow(IStoryProject &project);
    ~ResourcesWindow();
    virtual void Draw() override;

private:
    IStoryProject &m_project;

    bool m_showImportDialog{false};
    bool m_soundFile{false};

    void ChooseFile();
};
