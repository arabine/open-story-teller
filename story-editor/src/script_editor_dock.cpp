#include "script_editor_dock.h"

static const std::string test1 = R"(
; jump over the data, to our entry label
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


    ; Syscall test: show image and play sound
    lcons r0, $imageBird ; image name address in ROM located in R0 (null terminated)
    lcons r1, $soundChoice ; set to 0 if no sound
    syscall 1

    mov   r1, sp  ; save sp address in R1 (first element)
    lcons r0, .MEDIA_02
    push r0
    lcons r0, .MEDIA_03
    push r0
    lcons r2, 3 ; 3 iterations
    jump .media

; Generic media choice manager
.media:
    ; Les adresses des différents medias sont dans la stack
; Arguments:
    ; r1: address of the first media address
    ; r2: nombre d'itérations

; Local:
    ; t0: loop counter
    ; t1: increment 1
    ; t2: increment 4
    ; t4: current media address

.media_loop_start:
    mov t0, r2 ; i = 3
    lcons t1, 1
    lcons t2, 4
    mov t4, r1  ; copy address, r4 will be modified
.media_loop:
    sub t0, t1  ; i--
    add t4, t3  ; @++
    skipnz r0   ;  if (r0) goto start_loop;
    jump .media_loop_start
    push sp
    push r0
    push r1
    load r0, @r4, 4
    call r0
    pop r1
    pop r0
    pop sp

    ; TODO: wait for event

    jump .media_loop

.MEDIA_02:
    lcons r0, $imageBird ; image name address in ROM located in R0 (null terminated)
    lcons r1, $soundChoice ; set to 0 if no sound
    syscall 1
    ret

.MEDIA_03:
    lcons r0, $imageBird ; image name address in ROM located in R0 (null terminated)
    lcons r1, $soundChoice ; set to 0 if no sound
    syscall 1
    ret

.SYSCALL_TEST:
    ; syscall test: wait for event
    lcons r0, 0xFF  ; wait for all event, blocking
    syscall 2

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
