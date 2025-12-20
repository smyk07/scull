#include "backend/ld_utils.h"

#include "utils.h"
#include <stdio.h>

#include <stdlib.h>

void ld_link(const char *sysroot, const char *target_triple,
             const char *output_file, const char *obj_file) {
  char command[1024];
  snprintf(command, sizeof(command),
           "clang -static --target=%s --sysroot=%s -o %s %s > /dev/null",
           target_triple, sysroot, output_file, obj_file);
  int result = system(command);
  if (result != 0) {
    scu_perror(NULL, "Linking failed with code %d\n", result);
    exit(1);
  }
}
