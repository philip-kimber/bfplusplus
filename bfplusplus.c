
#include <stdio.h>
#include <stdlib.h>

#include "bfplusplus.h"

static void vm_grow_tape(BFVM* vm) {
  size_t old_len = vm->tape_length;
  size_t new_len = (size_t) old_len * TAPE_GROW_RATE;
  if (new_len > TAPE_MAX_LENGTH) {
    new_len = TAPE_MAX_LENGTH;
  }

  size_t tp_offset = vm->ptr - vm->tape;

  vm->tape_length = new_len;
  vm->tape = (BFCell*) realloc(vm->tape, sizeof(BFCell) * vm->tape_length);
  if (vm->tape == NULL) {
    throw_fault("not enough memory for cell access");
  }

  for (BFCell* local = vm->tape + old_len; local<vm->tape + vm->tape_length; local++) {
    local->type = TYPE_VALUE;
    local->as.VALUE = 0;
  }

  vm->ptr = vm->tape + tp_offset;
}

int vm_run(BFVM* vm) {

#define IP (vm->ip)
#define VALIDIP (IP < vm->instructions.insts + vm->instructions.length && IP >= vm->instructions.insts)
#define INST (*IP)
#define TP (vm->ptr)
#define VALIDTP (TP < vm->tape + TAPE_MAX_LENGTH && TP >= vm->tape)
#define NEED_GROWTH (TP >= vm->tape + vm->tape_length)
#define CELL (*TP)
#define ISVALUE (CELL.type == TYPE_VALUE)
#define ISZERO (ISVALUE && CELL.as.VALUE == 0)

  BFInst** loop_stack = NULL;
  size_t loop_stack_length = 0;

#define PUSH_LS(iptr) \
  do { \
    loop_stack_length++; \
    loop_stack = (BFInst**) realloc(loop_stack, sizeof(BFInst*) * loop_stack_length); \
    if (loop_stack == NULL) { throw_fault("maximum possible loop depth exceeded"); } \
    loop_stack[loop_stack_length-1] = iptr; \
  } while (0)
#define READ_LS (loop_stack[loop_stack_length-1])
#define SHRINK_LS \
  do { \
    loop_stack_length--; \
    if (loop_stack_length > 0) { \
      loop_stack = (BFInst**) realloc(loop_stack, sizeof(BFInst*) * loop_stack_length); \
    } \
    else { \
      free(loop_stack); \
      loop_stack = NULL; \
    } \
  } while (0)

  while (VALIDIP) {

    switch (INST) {

      case INST_PLUS:
        if (!ISVALUE) { throw_fault("+ operation not valid on function"); }
        CELL.as.VALUE++;
        break;

      case INST_MINUS:
        if (!ISVALUE) { throw_fault("- operation not valid on function"); }
        CELL.as.VALUE--;
        break;

      case INST_MOVE_LEFT:
        TP--;
        if (!VALIDTP) { throw_fault("< operator took pointer beyond valid region"); }
        break;

      case INST_MOVE_RIGHT:
        TP++;
        if (!VALIDTP) { throw_fault("> operator took pointer beyond valid region"); }
        if (NEED_GROWTH) {
          vm_grow_tape(vm);
        }
        break;

      case INST_OPEN_LOOP: {
        if (!ISZERO) {
          PUSH_LS(IP);
        }
        else {
          IP++;
          size_t loop_depth = 0;
          while (!(INST == INST_CLOSE_LOOP && loop_depth == 0)) {
            if (INST == INST_OPEN_LOOP) { loop_depth++; }
            else if (INST == INST_CLOSE_LOOP) { loop_depth--; }
            IP++;
            if (!VALIDIP) { throw_fault("mismatched loop brackets"); }
          }
        }
      } break;

      case INST_CLOSE_LOOP: {

        if (!ISZERO) {
          if (loop_stack_length == 0) { throw_fault("mismatched loop brackets"); }
          IP = READ_LS;
        }
        else {
          SHRINK_LS;
        }
      } break;



      case INST_GET_CHAR:
        if (!ISVALUE) { throw_fault(", operation not valid on function"); }
        CELL.as.VALUE = b_getchar();
        break;

      case INST_PUT_CHAR:
        if (!ISVALUE) { throw_fault(". operation not valid on function"); }
        b_putchar(CELL.as.VALUE);
        break;

    }
    IP++;

  }

  if (loop_stack_length != 0) {
    free(loop_stack);
    throw_fault("mismatched loop brackets");
  }

  return 0;
#undef IP
#undef VALIDIP
#undef INST
#undef TP
#undef VALIDTP
#undef NEED_GROWTH
#undef CELL
#undef ISZERO
#undef PUSH_LS
#undef READ_LS
#undef SHRINK_LS
}
