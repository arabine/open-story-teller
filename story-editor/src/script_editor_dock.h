#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QDockWidget>
#include "code_editor.h"
#include "highlighter.h"

class ScriptEditorDock : public QDockWidget
{
    Q_OBJECT

public:
    ScriptEditorDock();

    void HighlightLine(int line);

    void setScript(const std::string &script);
    QString getScript() const;

private:
    CodeEditor *m_editor{nullptr};
    Highlighter *m_highlighter;
};

#endif // SCRIPTEDITOR_H
