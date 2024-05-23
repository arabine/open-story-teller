#pragma once

#include "window_base.h"
#include "i_story_manager.h"
#include "gui.h"

class CpuWindow : public WindowBase
{
public:
    CpuWindow(IStoryManager &proj);

    void Initialize();
    virtual void Draw() override;

private:
    IStoryManager &m_story;

};

