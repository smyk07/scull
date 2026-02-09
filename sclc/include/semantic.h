/*
 * semantic: semantic checking for the simple-compiler. Includes variable
 * checking and type checking.
 */

#ifndef SEMANTIC
#define SEMANTIC

#include "ast.h"
#include "ds/dynamic_array.h"
#include "ds/ht.h"

#include <stddef.h>

/*
 * @brief: evaluate a constant expression to extract integer value
 *
 * @param expr: pointer to an expr_node
 *
 * @return: the integer value of the constant expression
 */
int evaluate_const_expr(expr_node *expr);

/*
 * @brief: go through all the variables and labels in the parse tree and check
 * for any erorrs.
 *
 * @param instrs: pointer to the dynamic_array of instructions.
 * @param variables: pointer to hash table of variable.
 * @param functions: pointer to the functions hash table.
 */
void check_semantics(dynamic_array *instrs, ht *variables, ht *functions);

#endif // !SEMANTIC
