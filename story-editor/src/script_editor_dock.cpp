#include "script_editor_dock.h"

static const std::string test1 = R"(; jump over the data, to our entry label
    jump         .entry

; Constant elements are separated by commas
$imageBird          DC8  "example.bmp", 8  ; string of chars, followed by one byte
$soundChoice        DC8  "choose1.snd"
$someConstant       DC32  12456789

; DVsxx to declare a variable in RAM, followed by the number of elements
$MyArray            DV8    10 ; array of 10 bytes
$RamData1           DV32    1 ; one 32-bit integer

; label definition
.entry:                 ;; comment here should work


    ; Syscall test (show image)
    lcons r0, $imageBird ; image name address in ROM located in R0 (null terminated)
    lcons r1, $soundChoice ; set to 0 if no sound
    syscall 1

; We create a stupid loop just for RAM variable testing

    lcons r0, 4 ; prepare loop: 4 iterations
    lcons r6, $RamData1 ; store address to R6
    store @r6, r0, 4    ; save R0 in RAM
    lcons r1, 1
.loop:
    load r0, @r6, 4     ; load this variable
    sub r0, r1
    store @r6, r0, 4    ; save R0 in RAM
    skipz r0            ; skip loop if R0 == 0
    jump .loop


    mov      r0, r2     ; copy R2 into R0 (blank space between , and R2)
mov R0,R2               ; copy R2 into R0 (NO blank space between , and R2)

    halt
)";


ScriptEditorDock::ScriptEditorDock()
    : QDockWidget(tr("Script editor"))
{
    setObjectName("ScriptEditorDock"); // used to save the state

    m_editor = new CodeEditor(this);

    m_editor->setPlainText(test1.c_str());
    m_highlighter = new Highlighter(m_editor->document());
    setWidget(m_editor);
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
