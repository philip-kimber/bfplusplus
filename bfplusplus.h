
#ifndef BF_PLUS_PLUS_H
#define BF_PLUS_PLUS_H

#include <stdint.h>

#define DEBUG_MLTRACK

#ifdef DEBUG_MLTRACK
#define MLTRACK_OVERRIDE
#include "mltrack/mltrack.h"
#endif

typedef enum _BFInst BFInst;
typedef struct _BFInstructions BFInstructions;
typedef struct _BFCell BFCell;
typedef struct _BFInstructions BFFn;
typedef struct _BFVM BFVM;
typedef struct _BFCall BFCall;

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
  INST_GET_CHAR,    /* , */
  INST_PUT_CHAR,    /* . */
};
struct _BFInstructions {
  BFInst* insts;
  size_t length;
};
struct _BFCell {
  enum {
    TYPE_VALUE,
    TYPE_FN,
  } type;

  union {
    BFFn* FN;
    uint16_t VALUE;
  } as;
};

#define TAPE_MAX_LENGTH 30000
#define TAPE_INITIAL_LENGTH 100
#define TAPE_GROW_RATE 1.5

struct _BFVM {
  BFCell* tape;
  uint16_t tape_length;
  BFCell* ptr;

  BFInstructions instructions;
  BFInst* ip;

  BFVM* parent;
};

struct _BFCall {
  BFCell* arguments;
  uint16_t arg_count;

  BFFn* fn;

  BFCell* results;
  uint16_t res_count;
};

/* ---- */

/* lexer.c */
int lex_file(const char* fpath, BFVM* vm);

/* utils.c */
void throw_fault(const char* msg);
void cell_destroy(BFCell cell);
void cells_dump(BFVM* vm);
uint16_t b_getchar();
void b_putchar(uint16_t c);

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
int vm_run(BFVM* vm);


#endif /* BF_PLUS_PLUS_H */
