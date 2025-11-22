/* c_to_asm.c
   Generador simple de ASM para el ejemplo factorial recursivo.
   Compilar: gcc -Wall -O2 c_to_asm.c -o c_to_asm
   Uso: ./c_to_asm programa.asm
*/

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr,"Usage: %s out.asm\n", argv[0]); return 1; }
    FILE *f = fopen(argv[1],"w");
    if (!f) { perror("open"); return 1; }

    fprintf(f, "; Programa ASM: factorial recursivo\n");
    fprintf(f, "; Convencion: caller pone argumento en ACC y hace PUSH; CALL fact; resultado en ACC\n\n");

    fprintf(f, "    ; main: carga n y llama a factorial\n");
    fprintf(f, "start:\n");
    fprintf(f, "    LDI 6        ; n = 6\n");
    fprintf(f, "    PUSH\n");
    fprintf(f, "    CALL fact\n");
    fprintf(f, "    OUT          ; imprime resultado\n");
    fprintf(f, "    HALT\n\n");

    fprintf(f, "; factorial(n): if n == 0 return 1;\n");
    fprintf(f, "fact:\n");
    fprintf(f, "    POP          ; recuperar argumento en ACC\n");   // ACC = n
    fprintf(f, "    STORE n_loc  ; guardar n en memoria\n");
    fprintf(f, "    LOAD n_loc\n");
    fprintf(f, "    LDI 0\n");
    fprintf(f, "    SUB n_loc    ; ACC = n - MEM[n_loc] ??? -> better do: compare n with 0\n");
    // The assembler lacks compare instruction; we'll simulate:
    // We'll implement: LOAD n_loc ; LDI 0 ; SUB tmp? Better: check if n == 0 via LOAD n_loc ; LDI 0 ; SUB result in ACC -> then JZ
    fprintf(f, "    LOAD n_loc\n");
    fprintf(f, "    LDI 0\n");
    fprintf(f, "    SUB n_loc\n");
    fprintf(f, "    JZ base_case\n");
    fprintf(f, "    ; recursive case: compute n-1, call fact, then multiply by n\n");
    fprintf(f, "    LOAD n_loc\n");
    fprintf(f, "    LDI 1\n");
    fprintf(f, "    SUB n_loc    ; ACC = n - 1  (we'll store it in arg_loc)\n");
    fprintf(f, "    STORE arg_loc\n");
    fprintf(f, "    LOAD arg_loc\n");
    fprintf(f, "    PUSH\n");
    fprintf(f, "    CALL fact\n");
    fprintf(f, "    ; now ACC has factorial(n-1)\n");
    fprintf(f, "    STORE tmp_res ; guardar res parcial\n");
    fprintf(f, "    LOAD n_loc\n");
    fprintf(f, "    MUL tmp_res   ; ACC = n * res\n");
    fprintf(f, "    RET\n");
    fprintf(f, "base_case:\n");
    fprintf(f, "    LDI 1\n");
    fprintf(f, "    RET\n\n");

    fprintf(f, "; datos\n");
    fprintf(f, "n_loc: .word 0\n");
    fprintf(f, "arg_loc: .word 0\n");
    fprintf(f, "tmp_res: .word 0\n");

    fclose(f);
    printf("ASM generado en %s\n", argv[1]);
    return 0;
}
