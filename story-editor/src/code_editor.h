#pragma once

#include "TextEditor.h"
#include "window_base.h"
#include "i_story_manager.h"

class CodeEditor : public WindowBase
{
public:
    CodeEditor(IStoryManager &project);

    virtual void Draw() override;

    void Initialize();

    void SetScript(const std::string &txt);
    std::string GetScript() const;
    void ClearErrors();
    void AddError(int line, const std::string &text);

    void HighlightLine(int line)
    {
        mEditor.SetExecutionMarker(line);
    }
private:
    IStoryManager &m_storyManager;

    TextEditor mEditor;
    TextEditor::Breakpoints m_breakPoints;
    TextEditor::ErrorMarkers m_markers;
};
