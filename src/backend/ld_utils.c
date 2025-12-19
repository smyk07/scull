#include "backend/ld_utils.h"

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

void ld_link(const char *output_file, const char *obj_file) {
  char command[512];
  snprintf(command, sizeof(command), "clang -o %s %s > /dev/null", output_file,
           obj_file);
  int result = system(command);
  if (result != 0) {
    scu_perror(NULL, "Linking failed with code %d\n", result);
    exit(1);
  }
}
