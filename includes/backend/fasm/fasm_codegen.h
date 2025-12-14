/*
 * codegen: converts the simple-compiler AST to Assembly.
 */

#ifndef FASM_CODEGEN_H
#define FASM_CODEGEN_H

#include "ast.h"
#include "ds/ht.h"
#include "ds/stack.h"

/*
 * @brief: convert a dynamic_array of instructions to FASM assembly.
 *
 * @param program: basically a wrapper around a dynamic_array of instructions.
 * @param variables: hash table of variables.
 * @param functions: pointer to the functions hash table.
 * @param filename: filename needed for output file.
 * @param errors: counter variable to increment when an error is encountered.
 */
void instrs_to_fasm(program_node *program, ht *variables, stack *loops,
                    ht *functions, unsigned int *errors);

#endif // !FASM_CODEGEN_H
