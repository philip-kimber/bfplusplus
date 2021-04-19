
#ifndef BF_PLUS_PLUS_H
#define BF_PLUS_PLUS_H

#include <stdint.h>

/*
 * Define to print a summary of any memory leaks, provided that mltrack.c and
 * mltrack.h are available in /mltrack dir
 */
/*#define DEBUG_MLTRACK*/
#ifdef DEBUG_MLTRACK
#define MLTRACK_OVERRIDE
#include "mltrack/mltrack.h"
#endif

/*
 * BF++ uses unsigned 16-bit type as its standard value type - requires a
 * <stdint.h> implementation as included above, and for the architecture to
 * provide a 16-bit type
 */
typedef uint16_t size_bf;

typedef enum _BFInst BFInst;
typedef struct _BFInstructions BFInstructions;
typedef struct _BFCell BFCell;
typedef struct _BFInstructions BFFn;
typedef struct _BFVM BFVM;
typedef struct _BFCall BFCall;

/*
 * The valid BF++ instructions
 */
enum _BFInst {
  INST_PLUS = 0,    /* + */
  INST_MINUS,       /* - */
  INST_MOVE_LEFT,   /* < */
  INST_MOVE_RIGHT,  /* > */
  INST_OPEN_LOOP,   /* [ */
  INST_CLOSE_LOOP,  /* ] */
  INST_OPEN_FN,     /* { */
  INST_CLOSE_FN,    /* } */
  INST_OPEN_CALL,   /* ( */
  INST_CLOSE_CALL,  /* ) */
  INST_SCOPE_UP,    /* ' */
  INST_SCOPE_GLOBAL,/* @ */
  INST_GET_CHAR,    /* , */
  INST_PUT_CHAR,    /* . */
};

/*
 * Structure for array of instructions to be fed to a VM.
 *
 * Note that BFFn is also a typedef for this struct, because function
 * definitions are just an array of instructions within.
 */
struct _BFInstructions {
  BFInst* insts;
  size_t length;
};

/*
 * Each cell can be a function or a value, and tagged by 'type' enum, and
 * accessed through 'as' union.
 *
 * This may be inefficient given just two types
 */
struct _BFCell {
  enum {
    TYPE_VALUE,
    TYPE_FN,
  } type;

  union {
    BFFn* FN;
    size_bf VALUE;
  } as;
};

/*
 * The maximum tape length is by default set to the canonical 30,000, but could
 * be set to anything below the unsigned 16-bit max of 65,535.
 *
 * The initial length is default 100, but could be anything between 1 and the
 * max length, affecting performance. Note that this will result in an
 * allocation of sizeof(BFCell) * TAPE_INITIAL_LENGTH every time a function is
 * called as each call has its own tape.
 *
 * The tape grow rate is set to 1.5 for no real reason - this is the amount
 * by which the length will be multiplied by when growing the tape. It is just
 * an array, so if realloc is fast, no real need to have a particularly high
 * grow rate.
 */
#define TAPE_MAX_LENGTH 30000
#define TAPE_INITIAL_LENGTH 100
#define TAPE_GROW_RATE 1.5

/*
 * The BF++ VM from which everything is run. The name may not be particularly
 * apt given that a new one is created for each function call.
 *
 * For function calls, the VM retains a reference to its parent VM which allows
  * for ' and @ specifiers to work.
 */
struct _BFVM {
  BFCell* tape;
  size_bf tape_length;
  BFCell* ptr;

  BFInstructions instructions;
  BFInst* ip;

  BFVM* parent;
};

/*
 * A struct used for function calls; stores the arguments and results as well as
 * the original fn definition object
 */
struct _BFCall {
  BFCell* arguments;
  size_bf arg_count;

  BFFn* fn;

  BFCell* results;
  size_bf res_count;
};

/* ---- */

/* lexer.c */
int lex_file(const char* fpath, BFVM* vm);

/* utils.c */
void throw_fault(const char* msg);
void cell_destroy(BFCell cell);
void cells_dump(BFVM* vm);
size_bf b_getchar();
void b_putchar(size_bf c);

BFFn* fn_create();
void fn_destroy(BFFn* fn);
void instructions_copy(BFInstructions* dest, BFInstructions* src);
BFCell cell_copy(BFCell c);

/* bfvm.c */
BFVM* vm_create();
void vm_destroy(BFVM* vm);

/* bfplusplus.c */
void vm_grow_tape(BFVM* vm);
void run_function_call(BFVM* vm, BFCall* call);
void vm_run(BFVM* vm);


#endif /* BF_PLUS_PLUS_H */
