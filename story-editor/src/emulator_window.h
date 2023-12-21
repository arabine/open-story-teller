#pragma once

#include "window_base.h"
#include "i_story_project.h"

class EmulatorWindow : public WindowBase
{
public:
    EmulatorWindow(IStoryProject &proj);

    void Initialize();
    virtual void Draw() override;

private:
    IStoryProject &m_project;
};

