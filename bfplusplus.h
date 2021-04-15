
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
typedef struct _BFFn BFFn;
typedef struct _BFVM BFVM;

enum _BFInst {
  INST_PLUS = 0,    /* + */
  INST_MINUS,       /* - */
  INST_MOVE_LEFT,   /* < */
  INST_MOVE_RIGHT,  /* > */
  INST_OPEN_LOOP,   /* [ */
  INST_CLOSE_LOOP,  /* ] */
  INST_OPEN_FN,     /* { */
  INST_CLOSE_FN,    /* } */
  INST_GET_CHAR,    /* , */
  INST_PUT_CHAR,    /* . */
};

struct _BFInstructions {
  BFInst* insts;
  int length;
};

struct _BFCell {
  enum {
    TYPE_BYTE,
    TYPE_FN,
  } type;

  union {
    BFFn* FN;
    uint8_t BYTE;
  } as;
};

struct _BFFn {
  BFInst* insts;
  int length;
};

#define VM_NO_CELLS 300

struct _BFVM {
  BFCell cells[VM_NO_CELLS];
  BFCell* ptr;

  BFInstructions instructions;
};

/* ---- */

int lex_file(const char* fpath, BFInstructions* dest);
void throw_fault(const char* msg);

BFVM* vm_create();
void vm_destroy(BFVM* vm);

void fn_destroy(BFFn* fn);
void cell_destroy(BFCell cell);

int vm_run(BFVM* vm);

void cells_dump(BFVM* vm);


#endif /* BF_PLUS_PLUS_H */
