
#include <stdio.h>
#include <stdlib.h>

#include "bfplusplus.h"

void throw_fault(const char* msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
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

void cells_dump(BFVM* vm) {
  for (int i=0; i<vm->tape_length; i++) {
    printf("Cell %d is type %d, as byte is %d\n", i, vm->tape[i].type, (int) vm->tape[i].as.VALUE);
  }
}
