#pragma once

#include "TextEditor.h"
#include "window_base.h"

class CodeEditor : public WindowBase
{
public:
    CodeEditor();

    virtual void Draw() override;

    void Initialize();
private:
    TextEditor mEditor;
    std::string mFileToEdit;
};
