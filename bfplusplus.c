
#include <stdio.h>
#include <stdlib.h>

#include "bfplusplus.h"

int lex_file(const char* fpath, BFInstructions* dest) {
#define ADD_INST(dest, inst) \
  do { \
    dest->length++; \
    dest->insts = (BFInst*) realloc(dest->insts, sizeof(BFInst) * dest->length); \
    dest->insts[dest->length-1] = inst; \
  } while (0)

  FILE* f = fopen(fpath, "r");
  if (f == NULL) {
    return -1;
  }

  int c;
  while ((c = getc(f)) != EOF) {
    switch ((char) c) {
      case '+':
        ADD_INST(dest, INST_PLUS);
        break;

      case '-':
        ADD_INST(dest, INST_MINUS);
        break;

      case '<':
        ADD_INST(dest, INST_MOVE_LEFT);
        break;

      case '>':
        ADD_INST(dest, INST_MOVE_RIGHT);
        break;

      case '[':
        ADD_INST(dest, INST_OPEN_LOOP);
        break;

      case ']':
        ADD_INST(dest, INST_CLOSE_LOOP);
        break;

      case '{':
        ADD_INST(dest, INST_OPEN_FN);
        break;

      case '}':
        ADD_INST(dest, INST_CLOSE_FN);
        break;

      case ',':
        ADD_INST(dest, INST_GET_CHAR);
        break;

      case '.':
        ADD_INST(dest, INST_PUT_CHAR);
        break;

      default:
        /* Ignore all other characters */
        break;
    }
  }

  return 0;
#undef ADD_INST
}
void throw_fault(const char* msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

BFVM* vm_create() {
  BFVM* out = (BFVM*) malloc(sizeof(BFVM));

  out->ptr = out->cells;
  for (int i=0; i<VM_NO_CELLS; i++) {
    out->cells[i].type = TYPE_BYTE;
    out->cells[i].as.BYTE = 0;
  }

  out->instructions.insts = NULL;
  out->instructions.length = 0;
}
void vm_destroy(BFVM* vm) {
  for (int i=0; i<VM_NO_CELLS; i++) {
    cell_destroy(vm->cells[i]);
  }

  free(vm->instructions.insts);
  free(vm);
}

void fn_destroy(BFFn* fn) {
  free(fn->insts);
  free(fn);
}
void cell_destroy(BFCell cell) {

  if (cell.type == TYPE_FN) {
    fn_destroy(cell.as.FN);
  }
}

int vm_run(BFVM* vm) {
  int ip = 0;
  int pos = 0;

  int* les = NULL; // Loop end stack
  int les_len = 0;

  while (ip < vm->instructions.length) {
#define INST vm->instructions.insts[ip]
#define CELL (*(vm->ptr))
    switch (INST) {

      case INST_PLUS:
        if (CELL.type != TYPE_BYTE) { throw_fault("+ operation not valid on function"); }
        CELL.as.BYTE++;
        ip++;
        break;

      case INST_MINUS:
        if (CELL.type != TYPE_BYTE) { throw_fault("+ operation not valid on function"); }
        CELL.as.BYTE--;
        ip++;
        break;

      case INST_MOVE_LEFT:
        pos--;
        if (pos < 0) { throw_fault("< operator took pointer beyond valid region"); }
        vm->ptr--;
        ip++;
        break;

      case INST_MOVE_RIGHT:
        pos++;
        if (pos > VM_NO_CELLS-1) { throw_fault("> operator took pointer beyond valid region"); }
        vm->ptr++;
        ip++;
        break;

      case INST_OPEN_LOOP: {
        if (CELL.type == TYPE_BYTE && CELL.as.BYTE != 0) {
          les_len++;
          les = (int*) realloc(les, sizeof(int) * les_len);
          les[les_len-1] = ip;
          ip++;
        }
        else {
          ip++;
          int loop_depth = 0;
          while (!(INST == INST_CLOSE_LOOP && loop_depth == 0)) {
            if (INST == INST_OPEN_LOOP) { loop_depth++; }
            else if (INST == INST_CLOSE_LOOP) { loop_depth--; }
            ip++;
          }
          ip++;
        }
      } break;

      case INST_CLOSE_LOOP: {
        int ret = les[les_len-1];

        if (CELL.type == TYPE_BYTE && CELL.as.BYTE != 0) {
          ip = ret;
        }
        else {
          les_len--;
          if (les_len > 0) {
            les = (int*) realloc(les, les_len-1);
          }
          else {
            free(les);
            les = NULL;
          }
        }
        ip++;
      } break;

      // todo more

      case INST_GET_CHAR:
        if (CELL.type != TYPE_BYTE) { throw_fault(", operation not valid on function"); }
        CELL.as.BYTE = (uint8_t) getchar();
        ip++;
        break;

      case INST_PUT_CHAR:
        if (CELL.type != TYPE_BYTE) { throw_fault(". operation not valid on function"); }
        putchar((int) CELL.as.BYTE);
        ip++;
        break;

    }
#undef INST
  }

  return 0;

}

void cells_dump(BFVM* vm) {
  for (int i=0; i<VM_NO_CELLS; i++) {
    printf("Cell %d is type %d, as byte is %d\n", i, vm->cells[i].type, (int) vm->cells[i].as.BYTE);
  }
}
