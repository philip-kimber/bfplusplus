
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bfplusplus.h"

/*
 * Exits with an error message.
 *
 * Quite a poor way of handling exceptions as it does not free/cleanup allocated
 * memory, but we assume the OS would do this.
 */
void throw_fault(const char* msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

/*
 * Safely destroy a cell, by destroying the fn object associated if it is a
 * function, otherwise does nothing.
 */
void cell_destroy(BFCell cell) {
  if (cell.type == TYPE_FN) {
    fn_destroy(cell.as.FN);
  }
}

/*
 * Prints the current state of the cells to the console.
 */
void cells_dump(BFVM* vm) {
  for (int i=0; i<vm->tape_length; i++) {
    if (vm->tape[i].type == TYPE_FN) {
      printf("Cell %d is FN with %I64d instructions\n", i, vm->tape[i].as.FN->length);
    }
    else {
      printf("Cell %d is VALUE %d\n", i, vm->tape[i].as.VALUE);
    }
  }
}

/*
 * Wrapper for getchar() that returns correct type and ensures correct handling
 * of EOF
 */
size_bf b_getchar() {
  int c = getchar();

  if (c == EOF || c == 0) {
    return 0;
  }
  else {
    unsigned char uc = (unsigned char) c;
    size_bf out = (size_bf) uc;
    return out;
  }

}

/*
 * Wrapper for putchar() that actually does very little, but performs correct
 * type casts
 */
void b_putchar(size_bf c) {
  unsigned char out = (unsigned char) c;
  putchar(out);
}

/*----*/

/*
 * Constructor and destructor for BFFn structure
 */
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

/*
 * Memcopy the instructions from one place to another, ie for loading a VM for
 * a function call
 */
void instructions_copy(BFInstructions* dest, BFInstructions* src) {
  dest->length = src->length;
  dest->insts = (BFInst*) malloc(dest->length * sizeof(BFInst));
  memcpy(dest->insts, src->insts, dest->length);
}

/*
 * Copy a cell (e.g for arguments and return values), simply returning a new
 * BFCell structure with the value if a value.
 * For function definitions (functions are first class), they are deep copied
 */
BFCell cell_copy(BFCell c) {
  BFCell out;
  if (c.type == TYPE_FN) {
    out.type = TYPE_FN;
    out.as.FN = fn_create();

    out.as.FN->length = c.as.FN->length;
    out.as.FN->insts = (BFInst*) malloc(sizeof(BFInst) * out.as.FN->length);
    memcpy(out.as.FN->insts, c.as.FN->insts, sizeof(BFInst) * out.as.FN->length);
  }
  else {
    out.type = TYPE_VALUE;
    out.as.VALUE = c.as.VALUE;
  }
  return out;
}
