#pragma once


#include "window_base.h"
#include "i_story_manager.h"
#include <unordered_set>

class DebuggerWindow : public WindowBase
{
public:
    DebuggerWindow(IStoryManager &project);

    virtual void Draw() override;

    void Initialize();

    void SetScript(const std::string &txt);
    std::string GetScript() const;
    void ClearErrors();
    void AddError(int line, const std::string &text);

    void HighlightLine(int line)
    {
        m_focusOnLine = line;
        // mEditor.SetExecutionMarker(line);
        // m_highlights[line] = ImVec4(0.5f, 0.5f, 1.0f, 0.5f);
    }
private:
    IStoryManager &m_storyManager;

    std::string m_text;
    std::unordered_set<int> m_breakpoints;
    // std::map<int, ImVec4> m_highlights;
    int m_focusOnLine{-1};
    int m_currentLine{-1};
    // TextEditor mEditor;
    // TextEditor::Breakpoints m_breakPoints;
    // TextEditor::ErrorMarkers m_markers;


    void TextViewDraw();
};
