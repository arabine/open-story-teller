#pragma once

#include "window_base.h"

class EmulatorWindow : public WindowBase
{
public:
    EmulatorWindow();

    void Initialize();
    virtual void Draw() override;

private:

};

