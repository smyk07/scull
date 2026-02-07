/*
 * cstate: Per-binary compilation state, groups all variables and options for
 * one build unit.
 *
 * Usage:
 * cstate *state = cstate_init(&cst, argc, argv);
 * ...
 * cstate_free(state);
 */

#ifndef CSTATE_H
#define CSTATE_H

#include "ds/arena.h"
#include "ds/dynamic_array.h"

#include <stdbool.h>
#include <stddef.h>

/*
 * @enum opt_level: represents optimization levels
 */
typedef enum opt_level {
  OPT_O0 = 0, // no optimization
  OPT_O1,     // basic
  OPT_O2,     // default
  OPT_O3,     // aggressive
  OPT_Os,     // optimize for size
  OPT_Oz      // optimize for minimum size
} opt_level;

/*
 * @struct coptions: represents the options described in the command when the
 * binary is executed.
 */
typedef struct coptions {
  /*
   * Print progress messages for various stages.
   */
  bool verbose;

  /*
   * Write output to output_filename instead of the default filename.
   */
  bool output;

  /*
   * Output object file only (compile but don't link)
   */
  bool compile_only;

  /*
   * Weather an include directory was specified in the command.
   */
  bool include_dir_specified;

  bool target_specified;

  /*
   * Emit LLVM IR
   */
  bool emit_llvm;

  /*
   * Emit target assembly
   */
  bool emit_asm;

  opt_level opt_level;
} coptions;

/*
 * @struct cstate: represents the compiler state.
 */
typedef struct cstate {
  /*
   * Arena for the fstates
   */
  mem_arena file_arena;

  /*
   * An array of fstates.
   */
  dynamic_array files;

  /*
   * State in which directory the scl files to be included are located
   */
  char *include_dir;

  /*
   * all the '.o' object files seperated with spaces
   */
  dynamic_array obj_file_list;

  /*
   * Path to the output file. default is extracted from the first filepath.
   * Ex: main.sclc => main
   */
  char *output_filepath;

  char *llvm_target_triple;

  /*
   * Options for the compilation process.
   */
  coptions options;
} cstate;

/*
 * @brief: Initialize a already allocated compiler state from CLI arguments.
 *
 * @param argc: count of args
 * @param argv: array of arguments (string)
 *
 */
void cstate_init(cstate *cst, int argc, char *argv[]);

/*
 * @brief: Free all associated memory of a compiler state
 *
 * @param s: pointer to malloc'd cstate.
 */
void cstate_free(cstate *s);

#endif // !CSTATE_H
