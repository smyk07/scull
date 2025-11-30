/*
 * fasm_utils: contains functions for the fasm assembler.
 */

#ifndef FASM_UTILS
#define FASM_UTILS

#include <stdio.h>

static FILE *fasm_output = NULL;

void init_fasm_output(const char *filename);

void emit(const char *format, ...);

void emit_without_indent(const char *format, ...);

void close_fasm_output();

/*
 * @brief: helper function to assemble the generated fasm assembly '.s' file to
 * an output object file.
 *
 * @param output_file: name to be given to the output object file.
 * @param asm_file: name of the generated assembly file.
 */
void fasm_assemble(const char *output_file, const char *asm_file);

#endif // !FASM_UTILS
