#include "fasm_utils.h"
#include "utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void init_fasm_output(const char *filename) {
  fasm_output = fopen(filename, "w");
  if (!fasm_output) {
    scu_perror(NULL, "Failed to open %s", filename);
    exit(1);
  }
}

void emit(const char *format, ...) {
  if (!fasm_output) {
    scu_perror(NULL, "Assembly output file not initialized\n");
    return;
  }
  va_list args;
  va_start(args, format);
  fprintf(fasm_output, "    ");
  vfprintf(fasm_output, format, args);
  va_end(args);
}

void emit_without_indent(const char *format, ...) {
  if (!fasm_output) {
    scu_perror(NULL, "Assembly output file not initialized\n");
    return;
  }
  va_list args;
  va_start(args, format);
  vfprintf(fasm_output, format, args);
  va_end(args);
}

void close_fasm_output() {
  if (fasm_output) {
    fclose(fasm_output);
    fasm_output = NULL;
  }
}

void fasm_assemble(const char *output_file, const char *asm_file) {
  char command[512];
  // might be dangerous :3
  snprintf(command, sizeof(command), "fasm %s %s.o > /dev/null", asm_file,
           output_file);
  int result = system(command);
  if (result != 0) {
    scu_perror(NULL, "Assembly failed with code %d\n", result);
    exit(1);
  }
}
