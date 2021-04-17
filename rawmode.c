
#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

static HWND console;
static DWORD old_console_mode;
static DWORD target_mode = ENABLE_INSERT_MODE | ENABLE_MOUSE_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_PROCESSED_OUTPUT;
static int mode_changed = 0;

int enter_raw_mode() {
  BOOL success;
  DWORD last_error;

  console = GetStdHandle(STD_INPUT_HANDLE);
  if (console == INVALID_HANDLE_VALUE) {
    last_error = GetLastError();
    printf("Error with GetStdHandle %08X\n", last_error);
    return 1;
  }

  success = GetConsoleMode(console, &old_console_mode);
  if (success == 0) {
    last_error = GetLastError();
    printf("Error with GetConsoleMode %08X\n", last_error);
    return 1;
  }

  success = SetConsoleMode(console, target_mode);
  if (success == 0) {
    last_error = GetLastError();
    printf("Error with GetConsoleMode %08X\n", last_error);
    return 1;
  }

  mode_changed = 1;

  return 0;
}

int exit_raw_mode() {
  BOOL success;
  DWORD last_error;

  if (mode_changed) {

    success = SetConsoleMode(console, old_console_mode);
    if (success == 0) {
      last_error = GetLastError();
      printf("Error with GetConsoleMode %08X\n", last_error);
      return 1;
    }

  }

  return 0;
}
