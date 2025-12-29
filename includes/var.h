/*
 * var: Contains all variable and related declarations.
 */

#ifndef VAR
#define VAR

#include "ds/ht.h"
#include <stddef.h>

/*
 * @enum type: represents data types.
 */
typedef enum type {
  TYPE_INT = 0,
  TYPE_CHAR,
  TYPE_STRING,
  TYPE_POINTER,
  TYPE_VOID
} type;

/*
 * @struct variable: represents a variable.
 */
typedef struct variable {
  type type;
  char *name;
  size_t line;
  size_t stack_offset;

  bool is_array;
  size_t dimensions;
  size_t *dimension_sizes;
} variable;

/*
 * @brief: get the size of a data type in bytes.
 *
 * @param t: data type.
 *
 * @return: size in bytes of the type t.
 */
int get_type_size(type t);

/*
 * @brief: check if a certain variable exists in a dynamic_array of variables.
 *
 * @param variables: pointer to hash table of variables.
 * @param var_to_find: pointer to a variable struct which we intend to find in
 * the dynamic_array.
 *
 * @return: int
 */
size_t get_var_stack_offset(ht *variables, variable *var_to_find);

/*
 * @brief: check for a variable's type by its name / identifier and line data.
 *
 * @param variables: pointer to hash table of variable.
 * @param var_to_find: pointer to a variable struct which we intend to find in
 * the dynamic_array.
 *
 * @return: data type of the variable (enumeration)
 */
type get_var_type(ht *variables, variable *var_to_find);

#endif // !VARE
