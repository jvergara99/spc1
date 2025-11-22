/* cpu_simulator.c
   Simulador Von Neumann simple (16-bit words).
   Compilar: gcc -Wall -O2 cpu_simulator.c -o cpu_simulator
   Uso: ./cpu_simulator programa.mem
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MEM_SIZE 4096

typedef uint16_t word;
word MEM[MEM_SIZE];

// Registros
uint16_t PC = 0;
uint16_t SP = MEM_SIZE; // stack grows downwards; SP points to next free
int32_t ACC = 0;        // accumulator: usamos 32-bit para evitar overflow simple
int32_t R1 = 0;
int ZF = 0; // zero flag

// Opcodes
enum {
    OP_NOP = 0,
    OP_HALT = 1,
    OP_LOAD = 2,
    OP_STORE = 3,
    OP_LDI = 4,
    OP_ADD = 5,
    OP_ADDI = 6,
    OP_SUB = 7,
    OP_JMP = 8,
    OP_JZ = 9,
    OP_JNZ = 10,
    OP_PUSH = 11,
    OP_POP = 12,
    OP_CALL = 13,
    OP_RET = 14,
    OP_MUL = 15,
    OP_OUT = 16
};

void load_mem(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) { perror("open mem"); exit(1); }
    char line[256];
    int idx = 0;
    while (fgets(line, sizeof(line), f)) {
        // allow comments and blank lines
        char *s = line;
        while (*s == ' ' || *s == '\t') s++;
        if (*s == '\0' || *s == '\n' || *s == ';') continue;
        unsigned int val;
        if (sscanf(s, "0x%x", &val) == 1 || sscanf(s, "%x", &val) == 1) {
            if (idx >= MEM_SIZE) { fprintf(stderr, "Program too large\n"); exit(1); }
            MEM[idx++] = (word)(val & 0xFFFF);
        } else {
            // try decimal
            if (sscanf(s, "%u", &val) == 1) {
                if (idx >= MEM_SIZE) { fprintf(stderr, "Program too large\n"); exit(1); }
                MEM[idx++] = (word)(val & 0xFFFF);
            }
        }
    }
    fclose(f);
}

void dump_state() {
    printf("[PC=%04u SP=%04u ACC=%d ZF=%d] ", PC, SP, (int)ACC, ZF);
}

int16_t sign_extend10(uint16_t x) {
    // operand is 10 bits; sign extend to 16 bits then to int16
    if (x & 0x200) { // bit 9 is sign
        return (int16_t)(0xFC00 | (x & 0x3FF));
    } else {
        return (int16_t)(x & 0x3FF);
    }
}

void run() {
    int steps = 0;
    while (1) {
        if (PC >= MEM_SIZE) { fprintf(stderr, "PC out of range: %u\n", PC); break; }
        word instr = MEM[PC++];
        uint16_t opcode = (instr >> 10) & 0x3F;
        uint16_t operand = instr & 0x3FF;
        // Trace
        dump_state();
        printf("FETCH instr=0x%04X (op=%u operand=%u)", instr, opcode, operand);
        printf("\n");
        steps++;
        switch (opcode) {
            case OP_NOP: break;
            case OP_HALT:
                printf("HALT encountered. Stopping.\n");
                return;
            case OP_LOAD:
                if (operand >= MEM_SIZE) { fprintf(stderr,"LOAD addr OOB\n"); return; }
                ACC = (int16_t)MEM[operand];
                ZF = (ACC == 0);
                break;
            case OP_STORE:
                if (operand >= MEM_SIZE) { fprintf(stderr,"STORE addr OOB\n"); return; }
                MEM[operand] = (word)(ACC & 0xFFFF);
                ZF = (ACC == 0);
                break;
            case OP_LDI: {
                int16_t imm = sign_extend10(operand);
                ACC = imm;
                ZF = (ACC == 0);
                break;
            }
            case OP_ADD:
                if (operand >= MEM_SIZE) { fprintf(stderr,"ADD addr OOB\n"); return; }
                ACC += (int16_t)MEM[operand];
                ZF = (ACC == 0);
                break;
            case OP_ADDI: {
                int16_t imm = sign_extend10(operand);
                ACC += imm;
                ZF = (ACC == 0);
                break;
            }
            case OP_SUB:
                if (operand >= MEM_SIZE) { fprintf(stderr,"SUB addr OOB\n"); return; }
                ACC -= (int16_t)MEM[operand];
                ZF = (ACC == 0);
                break;
            case OP_JMP:
                PC = operand;
                break;
            case OP_JZ:
                if (ZF) PC = operand;
                break;
            case OP_JNZ:
                if (!ZF) PC = operand;
                break;
            case OP_PUSH:
                if (SP == 0) { fprintf(stderr,"Stack overflow\n"); return; }
                SP--;
                MEM[SP] = (word)(ACC & 0xFFFF);
                break;
            case OP_POP:
                if (SP >= MEM_SIZE) { fprintf(stderr,"Stack underflow\n"); return; }
                ACC = (int16_t)MEM[SP];
                SP++;
                ZF = (ACC == 0);
                break;
            case OP_CALL:
                if (SP == 0) { fprintf(stderr,"Stack overflow\n"); return; }
                // push return address (current PC)
                SP--;
                MEM[SP] = (word)(PC & 0xFFFF);
                PC = operand;
                break;
            case OP_RET:
                if (SP >= MEM_SIZE) { fprintf(stderr,"Stack underflow\n"); return; }
                PC = MEM[SP];
                SP++;
                break;
            case OP_MUL:
                if (operand >= MEM_SIZE) { fprintf(stderr,"MUL addr OOB\n"); return; }
                ACC *= (int16_t)MEM[operand];
                ZF = (ACC == 0);
                break;
            case OP_OUT:
                printf("OUTPUT: %d\n", (int)ACC);
                break;
            default:
                fprintf(stderr,"Unknown opcode %u\n", opcode);
                return;
        }
        if (steps > 2000000) { fprintf(stderr,"Step limit exceeded\n"); return; }
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr,"Usage: %s programa.mem\n", argv[0]);
        return 1;
    }
    load_mem(argv[1]);
    printf("Memory loaded. Starting execution.\n");
    run();
    printf("Execution finished. Final ACC=%d PC=%u SP=%u\n", (int)ACC, PC, SP);
    return 0;
}
