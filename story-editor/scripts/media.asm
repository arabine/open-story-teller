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
 
