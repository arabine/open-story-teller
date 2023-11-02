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
    ; t4: where to jump when OK button is pressed
    ; t5: address of the "choice" array
    mov t5, r0  ; we save R0

.media_loop_start:
    load t0, @t5, 4 ; Le premier élément est le nombre de choix possibles, t0 = 3 (exemple)
    lcons t1, 1
    lcons t2, 4
    mov t3, t5
.media_loop:
    add t3, t2  ; @++

; --------- We call a media transition node
    load r0, @t3, 4 ; r0 =  content in ram at address in T3
    call r0 ; call subroutine
    
    ; Return argument in R0: the address of the node to call whe OK is pressed
    ; save it in t4
    mov t4, r0
    
    ; wait for event (OK or wheel)
    syscall 2
    ; Event is stored in R0
    
    ; -----  Test if event is OK button
    lcons r1, 1 ; mask for OK button
    and r1, r0 ; r1 = r1 AND r0
    skipz r1 ; not OK, skip jump
    jumpr t4 ; we do not plan to return here, so a jump is enough
    
    ; all other events mean: next node 

    sub t0, t1  ; i--
    skipnz t0   ;  if (r0) goto start_loop;
    jump .media_loop_start
    jump .media_loop
 
