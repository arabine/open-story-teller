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

; Liste des labels
$ChoiceObject       DC32,  .MEDIA_02, .MEDIA_03

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

    ; We put all the choices in reserved global memory
    lcons t2, 4 ; address increment
    lcons t0, $ChoiceMem
    lcons t1, .MEDIA_02
    store @t0, t1, 4    ; @t0 = t1

    lcons r0, .MEDIA_03
    push r0
    lcons r2, 2 ; 3 iterations
    jump .media ; no return possible, so a jump is enough

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
    skipnz t0   ;  if (r0) goto start_loop;
    jump .media_loop_start
    load r0, @t4, 4 ; r0 =  content in ram at address in T4
    call r0

    ; TODO: wait for event

    ; TODO: if ok event, free stack, then

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
