#pragma once

#include "i_story_manager.h"
#include "window_base.h"
#include "resource_manager.h"

class ResourcesWindow : public WindowBase
{
public:
    ResourcesWindow(ResourceManager &resources);
    ~ResourcesWindow();
    virtual void Draw() override;

private:
    ResourceManager &m_resources;

    bool m_showImportDialog{false};
    bool m_soundFile{false};

    void ChooseFile();
};
