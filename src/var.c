#include "var.h"
#include "utils.h"

type get_var_type(ht *variables, variable *var_to_find) {
  if (!variables || !var_to_find || !var_to_find->name)
    return -1;

  variable *var = ht_search(variables, var_to_find->name);

  if (!var) {
    scu_perror("Use of undeclared variable: %s [line %u]\n", var_to_find->name,
               var_to_find->line);
    return -1;
  }

  return var->type;
}

size_t get_var_stack_offset(ht *variables, variable *var_to_find) {
  if (!variables || !var_to_find || !var_to_find->name)
    return -1;

  variable *var = ht_search(variables, var_to_find->name);

  if (!var) {
    scu_perror("Use of undeclared variable: %s [line %u]\n", var_to_find->name,
               var_to_find->line);
    return -1;
  }

  return var->stack_offset;
}
