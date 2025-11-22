; Factorial iterativo 
; Calcula 6! = 720 y lo imprime con OUT

start:
    LDI 6
    STORE n_loc      ; n_loc = 6

    LDI 1
    STORE res        ; res = 1

    LOAD n_loc
    JZ done          ; si n == 0 saltar a done

loop:
    LOAD res         ; ACC = res
    MUL n_loc        ; ACC = res * n_loc
    STORE res        ; res = ACC

    LOAD n_loc       ; ACC = n
    SUB one          ; ACC = n - 1
    STORE n_loc      ; n = n - 1

    LOAD n_loc       ; ACC = n (actualizado)
    JNZ loop         ; si n != 0, repetir

done:
    LOAD res
    OUT
    HALT

; datos
n_loc:  .word 0
res:    .word 0
one:    .word 1
