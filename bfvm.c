
#include <stdio.h>
#include <stdlib.h>

#include "bfplusplus.h"

/*
 * Allocates a new VM structure and sets fields to initial values.
 * Instructions are initialised as blank; can be added by lexer
 */
BFVM* vm_create() {
  BFVM* out = (BFVM*) malloc(sizeof(BFVM));

  out->tape = (BFCell*) malloc(sizeof(BFCell) * TAPE_INITIAL_LENGTH);
  out->tape_length = TAPE_INITIAL_LENGTH;
  out->ptr = out->tape;
  for (int i=0; i<out->tape_length; i++) {
    out->tape[i].type = TYPE_VALUE;
    out->tape[i].as.VALUE = 0;
  }

  out->instructions.insts = NULL;
  out->instructions.length = 0;
  out->ip = NULL;

  out->parent = NULL;
  return out;
}

/*
 * Destroys, frees VM and all cells and instructions
 */
void vm_destroy(BFVM* vm) {
  for (int i=0; i<vm->tape_length; i++) {
    cell_destroy(vm->tape[i]);
  }
  free(vm->tape);

  free(vm->instructions.insts);
  free(vm);
}
