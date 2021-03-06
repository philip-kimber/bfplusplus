
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bfplusplus.h"
#include "rawmode.h"

int main(int argc, char** argv) {

#ifdef DEBUG_MLTRACK
  TRACK_init(stdout, TRACK_report_warn);
#endif

  char* fpath;
  if (argc == 1) {
    fpath = NULL;
    int len = 0;
    printf("Enter path to source file to run: ");
    int c;
    while ((c = getchar()) != '\n') {
      len++;
      fpath = (char*) realloc(fpath, len);
      fpath[len-1] = (char) c;
    }
    fpath = (char*) realloc(fpath, len+1);
    fpath[len] = '\0';
  }
  else if (argc == 2) {
    fpath = strdup(argv[1]);
  }

  BFVM* vm = vm_create();

  int res = lex_file(fpath, vm);
  if (res == -1) {
    printf("File at path %s not found\n", fpath);
    vm_destroy(vm);
    free(fpath);
    return 1;
  }
  free(fpath);

  enter_raw_mode();
  vm_run(vm);
  exit_raw_mode();
  printf("\n");

  vm_destroy(vm);

#ifdef DEBUG_MLTRACK
  TRACK_status(TRACK_print_chars);
#endif

  return 0;
}
