#pragma once

#include "window_base.h"
#include "i_story_manager.h"
#include "gui.h"

class VariablesWindow : public WindowBase
{
public:
    VariablesWindow(IStoryManager &proj);

    void Initialize();
    virtual void Draw() override;

private:
    IStoryManager &m_story;


    void ShowRAMEditor();

};

