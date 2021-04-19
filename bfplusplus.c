
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bfplusplus.h"

/*
 * Grows the VM's tape based on TAPE_GROW_RATE, making sure to preserve the
 * tape pointer in correct position in case realloc returns a different start.
 *
 * If the growth puts the VM beyond the TAPE_MAX_LENGTH, the length will be set
 * to TAPE_MAX_LENGTH, which the caller may need to check for.
 *
 * Throws a fault if realloc returns NULL.
 */
void vm_grow_tape(BFVM* vm) {
  size_bf old_len = vm->tape_length;
  size_bf new_len = (size_bf) old_len * TAPE_GROW_RATE;
  if (new_len > TAPE_MAX_LENGTH) {
    new_len = TAPE_MAX_LENGTH;
  }

  size_bf tp_offset = vm->ptr - vm->tape;

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

/*
 * Runs a function call in a new VM based on the values in the referenced BFCall
 * struct, essentially recursively because this calls vm_run() on the new VM.
 *
 * Sets the results in call->results to copied versions of the cells, so that
 * the call VM can be destroyed before returning
 */
void run_function_call(BFVM* vm, BFCall* call) {

  BFVM* callvm = vm_create();
  callvm->parent = vm;

  callvm->instructions.length = call->fn->length;
  callvm->instructions.insts = (BFInst*) malloc(sizeof(BFInst) * callvm->instructions.length);
  memcpy(callvm->instructions.insts, call->fn->insts, sizeof(BFInst) * callvm->instructions.length);

  callvm->ip = callvm->instructions.insts;

  for (int i=0; i<call->arg_count; i++) {
    *(callvm->ptr) = call->arguments[i];
    callvm->ptr++;
    if (callvm->ptr >= callvm->tape + callvm->tape_length) {
      vm_grow_tape(callvm);
    }
  }

  vm_run(callvm);

  for (int i=0; i<call->res_count; i++) {
    call->results[i] = cell_copy(*(callvm->ptr));

    callvm->ptr++;
    if (callvm->ptr >= callvm->tape + callvm->tape_length) {
      vm_grow_tape(callvm);
    }
  }

  vm_destroy(callvm);

}

/*
 * Runs a VM, executing the instructions, calling other functions etc.
 * The main function of the interpreter.
 *
 * Quite a simple yet slow implementation, iterating through the instructions
 * and switch based on different instructions. Maintains a loop_stack of return
 * addresses for loop ends which is manipulated by the PUSH_LS, READ_LS and
 * SHRINK_LS macros.
 */
void vm_run(BFVM* vm) {

/* Instruction pointer */
#define IP (vm->ip)

/* Is the instruction pointer within bounds? */
#define VALIDIP \
  (IP < vm->instructions.insts + vm->instructions.length \
    && IP >= vm->instructions.insts)

/* Current instruction */
#define INST (*IP)

/* Tape pointer */
#define TP (vm->ptr)

/* Is the tape pointer within bounds? */
#define VALIDTP \
  (TP < vm->tape + TAPE_MAX_LENGTH && TP >= vm->tape)

/* Does vm_grow_tape need to be called? */
#define NEED_GROWTH \
  (TP >= vm->tape + vm->tape_length)

/* Current cell */
#define CELL (*TP)

/* Is the current cell a value? */
#define ISVALUE (CELL.type == TYPE_VALUE)

/* Is the current cell 0? */
#define ISZERO (ISVALUE && CELL.as.VALUE == 0)

  BFInst** loop_stack = NULL;
  size_t loop_stack_length = 0;

/* Add a return instruction address for loop to the loop stack */
#define PUSH_LS(iptr) \
  do { \
    loop_stack_length++; \
    loop_stack = (BFInst**) realloc(loop_stack, sizeof(BFInst*) * loop_stack_length); \
    if (loop_stack == NULL) { throw_fault("maximum possible loop depth exceeded"); } \
    loop_stack[loop_stack_length-1] = iptr; \
  } while (0)

/* Get top of loop stack without removing it */
#define READ_LS (loop_stack[loop_stack_length-1])

/* Remove the top value of the loop stack, shrinking it */
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
          /* If not zero, run loop body as normal but push return address */
          PUSH_LS(IP);
        }
        else {
          /* If zero, skip over loop body */
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
          /* If not zero, go back to loop start */
          if (loop_stack_length == 0) { throw_fault("mismatched loop brackets"); }
          IP = READ_LS;
        }
        else {
          /* If zero, loop is finished; forget loop from stack */
          SHRINK_LS;
        }
      } break;

      case INST_OPEN_FN: {
        if (CELL.type == TYPE_FN) {
          /*
           * We can overwrite both functions and values with fn definitions
           * but fn definitions need to be freed/destroyed
           */
          fn_destroy(CELL.as.FN);
        }

        BFFn* fn = fn_create();

        /* Add loop body instructions to Fn structure at current position */
        IP++;
        size_t fn_depth = 0;
        while (!(INST == INST_CLOSE_FN && fn_depth == 0)) {
          if (INST == INST_OPEN_FN) { fn_depth++; }
          else if (INST == INST_CLOSE_FN) { fn_depth--; }

          fn->length++;
          fn->insts = (BFInst*) realloc(fn->insts, fn->length * sizeof(BFInst));
          fn->insts[fn->length-1] = INST;

          IP++;
          if (!VALIDIP) { throw_fault("mismatched function def brackets"); }
        }
        CELL.type = TYPE_FN;
        CELL.as.FN = fn;
      } break;

      case INST_OPEN_CALL: {
        BFCall call;

        /* Current cell should be value with function address index */
        if (CELL.type != TYPE_VALUE) { throw_fault("tried to call function with invalid address"); }
        size_bf fn_addr = CELL.as.VALUE;

        IP++;

        /*
         * Read through the instructions between the call brackets and discern
         * argument count, return count and scope decorator situation
         */
        size_t fn_sc = 0;
        int fn_sc_global = 0;
        call.arg_count = 0;
        call.res_count = 0;
        while (INST != INST_CLOSE_CALL) {
          switch (INST) {
            case INST_MINUS: /* - indicates number of spaces back to start pulling arguments from */
              call.arg_count++;
              break;

            case INST_PLUS: /* + indicates number of spaces forward to push results to */
              call.res_count++;
              break;

            case INST_SCOPE_UP: /* ' indicates number of scopes up to take the function from */
              fn_sc++;
              break;

            case INST_SCOPE_GLOBAL: /* @ indicates to take the function from the global scope */
              fn_sc_global = 1;
              break;

            default:
              /* Ignore other characters within call brackets */
              break;
          }
          IP++;
          if (!VALIDIP) { throw_fault("mismatched function call brackets"); }
        }

        /* Based on the scope decorators, get VM from which to find function */
        BFVM* fnvm = vm;
        if (fn_sc_global == 1) {
          while (fnvm->parent != NULL) {
            fnvm = fnvm->parent;
          }
        }
        else {
          for (int i=0; i<fn_sc; i++) {
            fnvm = vm->parent;
            if (fnvm == NULL) {
              throw_fault("invalid scope up configuration, nonexistent scope");
            }
          }
        }
        /* Set call structure function to function structure from address */
        if (fnvm->tape[fn_addr].type != TYPE_FN) { throw_fault("value at address for function call is not function"); }
        call.fn = fnvm->tape[fn_addr].as.FN;

        /* Push arguments */
        if (call.arg_count == 0) {
          call.arguments = NULL;
        }
        else {
          call.arguments = (BFCell*) malloc(sizeof(BFCell) * call.arg_count);
        }
        TP -= call.arg_count;
        if (!VALIDTP) { throw_fault("tried to push invalid number of arguments"); }
        for (int i=0; i<call.arg_count; i++) {
          call.arguments[i] = cell_copy(CELL);
          TP++;
        }

        /* Allocate result positions ready (arguably should be done in run_function_call) */
        if (call.res_count == 0) {
          call.results = NULL;
        }
        else {
          call.results = (BFCell*) malloc(sizeof(BFCell) * call.res_count);
        }

        /* Run the function */
        run_function_call(vm, &call);

        /* Pull the results */
        for (int i=0; i<call.res_count; i++) {
          TP++;
          if (!VALIDTP) { throw_fault("tried to pull invalid number of args"); }
          if (NEED_GROWTH) { vm_grow_tape(vm); }
          CELL = cell_copy(call.results[i]);
        }
        TP -= call.res_count;

        /* Free memory allocated for call args/return etc. */
        for (int i=0; i<call.arg_count; i++) {
          cell_destroy(call.arguments[i]);
        }
        free(call.arguments);

        for (int i=0; i<call.res_count; i++) {
          cell_destroy(call.results[i]);
        }
        free(call.results);

      } break;

      case INST_GET_CHAR:
        if (!ISVALUE) { throw_fault(", operation not valid on function"); }
        CELL.as.VALUE = b_getchar();
        break;

      case INST_PUT_CHAR:
        if (!ISVALUE) { throw_fault(". operation not valid on function"); }
        b_putchar(CELL.as.VALUE);
        break;

      case INST_SCOPE_UP:
      case INST_SCOPE_GLOBAL:
        /*
         * Do nothing, but no error: ' and @ characters allowed with no effect
         * outside call brackets
         */
        break;

      case INST_CLOSE_CALL:
      case INST_CLOSE_FN:
        /* Close instructions would always be handled by the open instructions */
        throw_fault("mismatched function/call end");
        break;

    }
    IP++;

  }

  if (loop_stack_length != 0) {
    /* If loop addresses still around and execution ended, must be mismatched */
    free(loop_stack);
    throw_fault("mismatched loop brackets");
  }

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
