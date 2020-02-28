#ifndef dictu_debug_h
#define dictu_debug_h

#include "chunk.h"

#ifndef DISASSEMBLE_FP
  #define DISASSEMBLE_FP stderr
#endif

void disassembleChunk(Chunk *chunk, const char *name);

int disassembleInstruction(Chunk *chunk, int offset);

#endif
