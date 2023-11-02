#pragma once

#include <vector>
#include <map>
#include <mutex>
#include "story_project.h"

class ResourcesWindow
{
public:
    ResourcesWindow(StoryProject &project);
    ~ResourcesWindow();
    void Draw(const char *title, bool *p_open);

private:
    StoryProject &m_project;

    bool m_showImportDialog{false};
    bool m_soundFile{false};

    void ChooseFile();
};
