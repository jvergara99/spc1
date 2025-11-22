; Programa ASM: factorial recursivo
; Convencion: caller pone argumento en ACC y hace PUSH; CALL fact; resultado en ACC

    ; main: carga n y llama a factorial
start:
    LDI 6        ; n = 6
    PUSH
    CALL fact
    OUT          ; imprime resultado
    HALT

; factorial(n): if n == 0 return 1;
fact:
    POP          ; recuperar argumento en ACC
    STORE n_loc  ; guardar n en memoria
    LOAD n_loc
    LDI 0
    SUB n_loc    ; ACC = n - MEM[n_loc] ??? -> better do: compare n with 0
    LOAD n_loc
    LDI 0
    SUB n_loc
    JZ base_case
    ; recursive case: compute n-1, call fact, then multiply by n
    LOAD n_loc
    LDI 1
    SUB n_loc    ; ACC = n - 1  (we'll store it in arg_loc)
    STORE arg_loc
    LOAD arg_loc
    PUSH
    CALL fact
    ; now ACC has factorial(n-1)
    STORE tmp_res ; guardar res parcial
    LOAD n_loc
    MUL tmp_res   ; ACC = n * res
    RET
base_case:
    LDI 1
    RET

; datos
n_loc: .word 0
arg_loc: .word 0
tmp_res: .word 0
