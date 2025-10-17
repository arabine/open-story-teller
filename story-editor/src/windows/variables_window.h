#pragma once

#include "window_base.h"
#include "i_story_project.h"
#include "gui.h"

class VariablesWindow : public WindowBase
{
public:
    VariablesWindow();

    void Initialize();
    void Draw() override;
    void Draw(std::shared_ptr<IStoryProject> story);

private:
    void DrawVariableEditor(std::shared_ptr<IStoryProject> story);
};

