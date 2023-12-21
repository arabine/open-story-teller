#pragma once

#include "TextEditor.h"
#include "window_base.h"

class CodeEditor : public WindowBase
{
public:
    CodeEditor();

    virtual void Draw() override;

    void Initialize();

    void SetScript(const std::string &txt);
    void ClearErrors();
    void AddError(int line, const std::string &text);
private:
    TextEditor mEditor;
    TextEditor::Breakpoints m_breakPoints;
    TextEditor::ErrorMarkers m_markers;
};
