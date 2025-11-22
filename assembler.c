/* assembler.c
   Ensamblador simple para la ISA usada por cpu_simulator.c
   Compilar: gcc -Wall -O2 assembler.c -o assembler
   Uso: ./assembler programa.asm programa.mem
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#define MAX_LINES 4096
#define MAX_LABELS 1024

typedef struct { char name[64]; int addr; } label_t;
label_t labels[MAX_LABELS];
int label_count = 0;

char *trim(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    char *end = s + strlen(s) - 1;
    while (end > s && (isspace((unsigned char)*end) || *end=='\r' || *end=='\n')) { *end = '\0'; end--; }
    return s;
}

int find_label(const char *name) {
    for (int i=0;i<label_count;i++) if (strcmp(labels[i].name, name)==0) return labels[i].addr;
    return -1;
}

void add_label(const char *name, int addr) {
    if (label_count >= MAX_LABELS) { fprintf(stderr,"Too many labels\n"); exit(1); }
    strcpy(labels[label_count].name, name);
    labels[label_count].addr = addr;
    label_count++;
}

int opcode_for(const char *m) {
    if (!strcmp(m,"NOP")) return 0;
    if (!strcmp(m,"HALT")) return 1;
    if (!strcmp(m,"LOAD")) return 2;
    if (!strcmp(m,"STORE")) return 3;
    if (!strcmp(m,"LDI")) return 4;
    if (!strcmp(m,"ADD")) return 5;
    if (!strcmp(m,"ADDI")) return 6;
    if (!strcmp(m,"SUB")) return 7;
    if (!strcmp(m,"JMP")) return 8;
    if (!strcmp(m,"JZ")) return 9;
    if (!strcmp(m,"JNZ")) return 10;
    if (!strcmp(m,"PUSH")) return 11;
    if (!strcmp(m,"POP")) return 12;
    if (!strcmp(m,"CALL")) return 13;
    if (!strcmp(m,"RET")) return 14;
    if (!strcmp(m,"MUL")) return 15;
    if (!strcmp(m,"OUT")) return 16;
    return -1;
}

int main(int argc, char **argv) {
    if (argc < 3) { fprintf(stderr,"Usage: %s in.asm out.mem\n", argv[0]); return 1; }
    FILE *fin = fopen(argv[1],"r");
    if (!fin) { perror("open asm"); return 1; }
    char *lines[MAX_LINES];
    int line_count = 0;
    char buf[512];
    while (fgets(buf,sizeof(buf),fin)) {
        lines[line_count] = strdup(buf);
        line_count++;
        if (line_count >= MAX_LINES) break;
    }
    fclose(fin);

    // Pass 1: labels and addresses
    int addr = 0;
    for (int i=0;i<line_count;i++) {
        char local[512];
        strcpy(local, lines[i]);
        char *s = trim(local);
        if (*s == ';' || *s == '\0') continue;
        // label?
        char *col = strchr(s, ':');
        if (col) {
            *col = 0;
            char labelname[64];
            sscanf(s, "%63s", labelname);
            add_label(labelname, addr);
            s = trim(col+1);
            if (*s == '\0') continue;
        }
        // directive .word counts as one word
        if (strncmp(s, ".word", 5) == 0) { addr++; continue; }
        // otherwise instruction -> one word
        addr++;
    }

    // Pass 2: translate
    FILE *fout = fopen(argv[2],"w");
    if (!fout) { perror("open out"); return 1; }
    addr = 0;
    for (int i=0;i<line_count;i++) {
        char local[512];
        strcpy(local, lines[i]);
        char *s = trim(local);
        if (*s == ';' || *s == '\0') continue;
        // handle label
        char *col = strchr(s, ':');
        if (col) { s = trim(col+1); if (*s == '\0') continue; }
        if (strncmp(s, ".word", 5) == 0) {
            char *p = s+5;
            p = trim(p);
            int val = 0;
            if (p[0]=='0' && p[1]=='x') sscanf(p, "%x", &val);
            else val = atoi(p);
            fprintf(fout, "0x%04X\n", val & 0xFFFF);
            addr++;
            continue;
        }
        // parse mnemonic and operand
        char mnem[64]; char opstr[128];
        int n = sscanf(s, "%63s %127[^\n\r;]", mnem, opstr);
        if (n < 1) continue;
        for (char *c=mnem; *c; ++c) *c = toupper((unsigned char)*c);
        int opc = opcode_for(mnem);
        if (opc < 0) { fprintf(stderr,"Unknown mnemonic %s at line %d\n", mnem, i+1); return 1; }
        int operand = 0;
        if (n == 2) {
            char *p = trim(opstr);
            // if label
            if (isalpha((unsigned char)p[0]) || p[0]=='_') {
                int la = find_label(p);
                if (la < 0) { fprintf(stderr,"Unknown label %s\n", p); return 1; }
                operand = la & 0x3FF;
            } else if (p[0]=='0' && (p[1]=='x' || p[1]=='X')) {
                unsigned int tmp; sscanf(p, "%x", &tmp); operand = tmp & 0x3FF;
            } else {
                int tmp = atoi(p); operand = tmp & 0x3FF;
            }
        }
        unsigned short instr = ((opc & 0x3F) << 10) | (operand & 0x3FF);
        fprintf(fout, "0x%04X\n", instr);
        addr++;
    }
    fclose(fout);
    printf("Assembled %s -> %s (labels=%d)\n", argv[1], argv[2], label_count);
    return 0;
}
