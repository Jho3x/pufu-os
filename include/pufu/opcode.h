#ifndef PUFU_OPCODE_H
#define PUFU_OPCODE_H

typedef enum {
  OP_NOP = 0x00,
  OP_MOV = 0x01,
  OP_ADD = 0x02,
  OP_SUB = 0x03,
  OP_MUL = 0x04,
  OP_DIV = 0x05,
  OP_CMP = 0x06,

  // Control Flow
  OP_JMP = 0x10,
  OP_BEQ = 0x11,
  OP_BNE = 0x12,
  OP_BLT = 0x13,
  OP_BGT = 0x14,
  OP_LABEL = 0x15, // Meta-instruction

  // System
  OP_SYSCALL = 0xFF
} PufuOpcode;

#endif // PUFU_OPCODE_H
