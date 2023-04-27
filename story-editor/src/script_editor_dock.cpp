#include "script_editor_dock.h"

static const std::string test1 = R"(
; jump over the data, to our entry label
    jump         .entry

; Constant elements are separated by commas
$imageBird          DC8, "example.bmp", 8  ; string of chars, followed by one byte
$soundChoice        DC8,  "choose1.snd"
$yaya               DC8,  "yaya.bmp"
$rabbit             DC8,  "rabbit.bmp"
$someConstant       DC32,  12456789

; Liste des noeuds à appeler
$ChoiceObject       DC32,  2, .MEDIA_02, .MEDIA_03

; DVsxx to declare a variable in RAM, followed by the number of elements
$MyArray            DV8,    10 ; array of 10 bytes
$RamData1           DV32,    1 ; one 32-bit integer
$ChoiceMem          DV32,    10 ; 10 elements for the choices, to be generated

; label definition
.entry:                 ;; comment here should work


    ; Syscall test: show image and play sound
    lcons r0, $imageBird ; image name address in ROM located in R0 (null terminated)
    lcons r1, $soundChoice ; set to 0 if no sound
    syscall 1
    lcons r0, $ChoiceObject
    jump .media ; no return possible, so a jump is enough

; Generic media choice manager
.media:
    ; Les adresses des différents medias sont dans la stack
; Arguments:
    ; r0: address d'une structure de type "media choice"
; Local:
    ; t0: loop counter
    ; t1: increment 1
    ; t2: increment 4
    ; t3: current media address

.media_loop_start:
    load t0, @r0, 4 ; Le premier élément est le nombre de choix possibles, t0 = 3 (exemple)
    lcons t1, 1
    lcons t2, 4
    mov t3, r0
.media_loop:
    add t3, t2  ; @++


; -------  On appelle un autre media node
    push r0 ; save r0
    load r0, @t3, 4 ; r0 =  content in ram at address in T4
    call r0
    pop r0
    ; TODO: wait for event

    sub t0, t1  ; i--
    skipnz t0   ;  if (r0) goto start_loop;
    jump .media_loop_start
    jump .media_loop

.MEDIA_02:
    lcons r0, $yaya ; image name address in ROM located in R0 (null terminated)
    lcons r1, $soundChoice ; set to 0 if no sound
    syscall 1
    ret

.MEDIA_03:
    lcons r0, $rabbit
    lcons r1, $soundChoice
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
