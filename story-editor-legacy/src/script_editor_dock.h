#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include "dock_widget_base.h"
#include "code_editor.h"
#include "highlighter.h"

class ScriptEditorDock : public DockWidgetBase
{
    Q_OBJECT

public:
    ScriptEditorDock();

    void HighlightLine(int line);

    void setScript(const std::string &script);
    QString getScript() const;

    void SetBreakPoints(const std::set<int> & bkp);

signals:
    void sigLineNumberAreaClicked(int line);

private:
    CodeEditor *m_editor{nullptr};
    Highlighter *m_highlighter;
};

#endif // SCRIPTEDITOR_H
