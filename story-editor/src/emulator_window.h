#pragma once

#include "window_base.h"
#include "i_story_manager.h"
#include "gui.h"

class EmulatorWindow : public WindowBase
{
public:
    EmulatorWindow(IStoryManager &proj);

    void Initialize();
    virtual void Draw() override;

    void ClearImage();
    void SetImage(const std::string &image);

private:
    IStoryManager &m_story;
    Gui::Image m_image;
    std::string m_imageFileName;
};

