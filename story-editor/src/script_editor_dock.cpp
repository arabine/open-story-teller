#include "script_editor_dock.h"


ScriptEditorDock::ScriptEditorDock()
    : DockWidgetBase(tr("Script editor"), false)
{
    setObjectName("ScriptEditorDock"); // used to save the state

    m_editor = new CodeEditor(this);
    m_highlighter = new Highlighter(m_editor->document());
    setWidget(m_editor);

    connect(m_editor, &CodeEditor::sigLineNumberAreaClicked, this, &ScriptEditorDock::sigLineNumberAreaClicked);
}

void ScriptEditorDock::HighlightLine(int line)
{
    m_highlighter->highlightLine(line);
}

void ScriptEditorDock::setScript(const std::string &script)
{
    m_editor->setPlainText(script.c_str());
}

QString ScriptEditorDock::getScript() const
{
    return m_editor->toPlainText();
}

void ScriptEditorDock::SetBreakPoints(const std::set<int> &bkp)
{
    m_editor->SetBreakPoints(bkp);
}
