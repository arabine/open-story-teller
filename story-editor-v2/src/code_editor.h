#pragma once

#include "TextEditor.h"
#include "window_base.h"

class CodeEditor : public WindowBase
{
public:
    CodeEditor();

    void Draw(const char *title, bool *p_open);

    void Initialize();
private:
    TextEditor mEditor;
    std::string mFileToEdit;
};
