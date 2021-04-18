
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bfplusplus.h"

void throw_fault(const char* msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

void cell_destroy(BFCell cell) {

  if (cell.type == TYPE_FN) {
    fn_destroy(cell.as.FN);
  }
}

void cells_dump(BFVM* vm) {
  for (int i=0; i<vm->tape_length; i++) {
    if (vm->tape[i].type == TYPE_FN) {
      printf("Cell %d is FN with %d instructions\n", i, vm->tape[i].as.FN->length);
    }
    else {
      printf("Cell %d is VALUE %d\n", i, vm->tape[i].as.VALUE);
    }
  }
}

uint16_t b_getchar() {
  int c = getchar();

  if (c == EOF || c == 0) {
    return 0;
  }
  else {
    unsigned char out = (unsigned char) c;
    uint16_t out16 = (uint16_t) out;
    return out16;
  }

}
void b_putchar(uint16_t c) {
  unsigned char out = (unsigned char) c;
  putchar(out);
}

/*----*/

BFFn* fn_create() {
  BFFn* out = (BFFn*) malloc(sizeof(BFFn));
  out->length = 0;
  out->insts = NULL;
  return out;
}
void fn_destroy(BFFn* fn) {
  free(fn->insts);
  free(fn);
}

void instructions_copy(BFInstructions* dest, BFInstructions* src) {
  dest->length = src->length;
  dest->insts = (BFInst*) malloc(dest->length * sizeof(BFInst));
  memcpy(dest->insts, src->insts, dest->length);
}

BFCell cell_copy(BFCell c) {
  BFCell out;
  if (c.type == TYPE_VALUE) {
    out.type = TYPE_VALUE;
    out.as.VALUE = c.as.VALUE;
    return out;
  }
  else if (c.type == TYPE_FN) {
    out.type = TYPE_FN;
    out.as.FN = fn_create();

    out.as.FN->length = c.as.FN->length;
    out.as.FN->insts = (BFInst*) malloc(sizeof(BFInst) * out.as.FN->length);
    memcpy(out.as.FN->insts, c.as.FN->insts, sizeof(BFInst) * out.as.FN->length);
    return out;
  }
}
