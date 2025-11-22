#!/usr/bin/env bash
if [ -z "$1" ]; then echo "Uso: $0 programa.mem"; exit 1; fi
awk '{
  line=NR-1;
  gsub(/^0x/,"",$1);
  v=strtonum("0x"$1);
  opcode=and(rshift(v,10),0x3F);
  operand=and(v,0x3FF);
  op="UNK";
  if (opcode==0) op="NOP";
  else if (opcode==1) op="HALT";
  else if (opcode==2) op="LOAD";
  else if (opcode==3) op="STORE";
  else if (opcode==4) op="LDI";
  else if (opcode==5) op="ADD";
  else if (opcode==6) op="ADDI";
  else if (opcode==7) op="SUB";
  else if (opcode==8) op="JMP";
  else if (opcode==9) op="JZ";
  else if (opcode==10) op="JNZ";
  else if (opcode==11) op="PUSH";
  else if (opcode==12) op="POP";
  else if (opcode==13) op="CALL";
  else if (opcode==14) op="RET";
  else if (opcode==15) op="MUL";
  else if (opcode==16) op="OUT";
  printf("%04d: 0x%04X  %-6s %4d\n", line, v, op, operand);
}' "$1"
