#pragma once

#include "window_base.h"

class EmulatorWindow : public WindowBase
{
public:
    EmulatorWindow();

    void Initialize();
    void Draw(const char* title, bool* p_open);

private:

};

