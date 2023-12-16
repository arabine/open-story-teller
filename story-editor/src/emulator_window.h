#pragma once

#include "window_base.h"
#include "gui.h"

class EmulatorWindow : public WindowBase
{
public:
    EmulatorWindow();

    void Initialize();
    virtual void Draw() override;

private:
    Gui::Image m_playImage;
    Gui::Image m_pauseImage;
    Gui::Image m_homeImage;

};

