
#include <stdio.h>
#include <stdlib.h>

#include "bfplusplus.h"

int lex_file(const char* fpath, BFVM* vm) {
#define ADD_INST(vm, inst) \
  do { \
    vm->instructions.length++; \
    vm->instructions.insts = (BFInst*) realloc(vm->instructions.insts, sizeof(BFInst) * vm->instructions.length); \
    vm->instructions.insts[vm->instructions.length-1] = inst; \
  } while (0)

  FILE* f = fopen(fpath, "r");
  if (f == NULL) {
    return -1;
  }

  int c;
  while ((c = getc(f)) != EOF) {
    switch ((char) c) {
      case '+':
        ADD_INST(vm, INST_PLUS);
        break;

      case '-':
        ADD_INST(vm, INST_MINUS);
        break;

      case '<':
        ADD_INST(vm, INST_MOVE_LEFT);
        break;

      case '>':
        ADD_INST(vm, INST_MOVE_RIGHT);
        break;

      case '[':
        ADD_INST(vm, INST_OPEN_LOOP);
        break;

      case ']':
        ADD_INST(vm, INST_CLOSE_LOOP);
        break;

      case '(':
        ADD_INST(vm, INST_OPEN_CALL);
        break;

      case ')':
        ADD_INST(vm, INST_CLOSE_CALL);
        break;

      case '{':
        ADD_INST(vm, INST_OPEN_FN);
        break;

      case '}':
        ADD_INST(vm, INST_CLOSE_FN);
        break;

      case ',':
        ADD_INST(vm, INST_GET_CHAR);
        break;

      case '.':
        ADD_INST(vm, INST_PUT_CHAR);
        break;

      default:
        /* Ignore all other characters */
        break;
    }
  }

  vm->ip = vm->instructions.insts;

  return 0;
#undef ADD_INST
}
