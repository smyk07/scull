#include "cstate.h"
#include "backend/backend.h"
#include "ds/dynamic_array.h"
#include "fstate.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

cstate *cstate_create_from_args(int argc, char *argv[]) {
  cstate *cst = scu_checked_malloc(sizeof(cstate));

  if (argc <= 1) {
    /*
     * Default output.
     * Should list out each option and a short description.
     * Take care of wrapping after ~80 characters.
     */
    printf("SCULL Compiler\n");
    printf("Usage: %s [OPTIONS] <input_files>\n\n", argv[0]);

    printf("OPTIONS:\n");

    printf("--target [TARGET]             OR  -t  Specify output target.\n");

    printf("--output <output_filename>    OR  -o  Specify output binary "
           "filename.\n");

    printf("--include_dir <path_to_dir>   OR  -i  Specify include directory "
           "path.\n");

    printf("--verbose                     OR  -v  Print debug messages\n");

    printf("--emit-llvm                           Emit LLVM IR\n");

    printf("\n");

    printf("Available Targets:\n");
    for (int i = 0; i <= TARGET_C; i++) {
      printf("%s ", target_kind_to_string((target_kind)i));
    }
    printf("\n");

    free(cst);
    exit(1);
  }

  int i = 1;
  dynamic_array filenames;
  dynamic_array_init(&filenames, sizeof(char *));
  cst->output_filepath = NULL;
  cst->include_dir = NULL;
  cst->error_count = 0;
  cst->target = TARGET_FASM;

  while (i < argc) {
    char *arg = argv[i];

    if (strcmp(arg, "--target") == 0 || strcmp(arg, "-t") == 0) {
      if (i + 1 >= argc) {
        scu_perror(&cst->error_count, "Missing target after %s\n", arg);
        exit(1);
      }

      char *target_str = argv[i + 1];
      bool valid_target = false;

      if (strcmp(target_str, TARGET_FASM_STR) == 0) {
        cst->target = TARGET_FASM;
        valid_target = true;
      } else if (strcmp(target_str, TARGET_C_STR) == 0) {
        cst->target = TARGET_C;
        valid_target = true;
      } else if (strcmp(target_str, TARGET_LLVM_X86_64_STR) == 0) {
        cst->target = TARGET_LLVM_X86_64;
        valid_target = true;
      } else if (strcmp(target_str, TARGET_LLVM_AARCH64_STR) == 0) {
        cst->target = TARGET_LLVM_AARCH64;
        valid_target = true;
      } else if (strcmp(target_str, TARGET_LLVM_RISCV64_STR) == 0) {
        cst->target = TARGET_LLVM_RISCV64;
        valid_target = true;
      }

      if (!valid_target) {
        scu_perror(&cst->error_count, "Invalid target: %s\n", target_str);
        exit(1);
      }

      i += 2;
      continue;
    }

    if (strcmp(arg, "--output") == 0 || strcmp(arg, "-o") == 0) {
      if (i + 1 >= argc) {
        scu_perror(&cst->error_count, "Missing filename after %s\n", arg);
        exit(1);
      }

      if (cst->output_filepath != NULL) {
        scu_perror(&cst->error_count, "Output specified more than once: %s\n",
                   argv[i + 1]);
        exit(1);
      }

      cst->output_filepath = strdup(argv[i + 1]);
      cst->options.output = true;
      i += 2;
      continue;
    }

    if (strcmp(arg, "--include_dir") == 0 || strcmp(arg, "-i") == 0) {
      if (i + 1 >= argc) {
        scu_perror(&cst->error_count, "Missing directory path after %s\n", arg);
        exit(1);
      }

      if (cst->include_dir != NULL) {
        scu_perror(&cst->error_count,
                   "Include directory specified more than once: %s\n",
                   argv[i + 1]);
        exit(1);
      }

      struct stat st;
      if (stat(argv[i + 1], &st) != 0) {
        scu_perror(&cst->error_count, "Include directory does not exist: %s\n",
                   argv[i + 1]);
        exit(1);
      }

      if (!S_ISDIR(st.st_mode)) {
        scu_perror(&cst->error_count, "Path is not a directory: %s\n",
                   argv[i + 1]);
        exit(1);
      }

      cst->include_dir = strdup(argv[i + 1]);
      cst->options.include_dir_specified = true;

      i += 2;
      continue;
    }

    if (strcmp(arg, "--verbose") == 0 || strcmp(arg, "-v") == 0) {
      cst->options.verbose = true;
      i++;
      continue;
    }

    if (strcmp(arg, "--emit-llvm") == 0) {
      cst->options.emit_llvm = true;
      i++;
      continue;
    }

    if (arg[0] != '-') {
      char *filename_copy = strdup(arg);
      dynamic_array_append(&filenames, &filename_copy);
      i++;
      continue;
    }

    scu_perror(&cst->error_count, "Unknown option: %s\n", arg);
    exit(1);
  }

  if (cst->include_dir == NULL)
    cst->include_dir = strdup(".");

  if (filenames.count == 0) {
    scu_perror(&cst->error_count, "Missing input filename\n");
    exit(1);
  }

  if (cst->output_filepath == NULL) {
    char *first_filename;
    dynamic_array_get(&filenames, 0, &first_filename);
    cst->output_filepath = scu_extract_name(first_filename);
    if (!cst->output_filepath) {
      scu_perror(&cst->error_count, "Failed to extract filename.\n");
      exit(1);
    }
  }

  dynamic_array_init(&cst->files, sizeof(fstate *));

  for (size_t i = 0; i < filenames.count; i++) {
    char *filepath;
    dynamic_array_get(&filenames, i, &filepath);

    fstate *fst = create_new_fstate(filepath);
    if (fst == NULL) {
      scu_perror(&cst->error_count, "Failed to create fstate for: %s\n",
                 filepath);
      exit(1);
    }

    dynamic_array_append(&cst->files, &fst);
    free(filepath);
  }

  dynamic_array_free(&filenames);

  return cst;
}

void cstate_free(cstate *s) {
  if (s == NULL)
    return;

  if (s->include_dir != NULL)
    free(s->include_dir);

  if (s->output_filepath != NULL)
    free(s->output_filepath);

  for (size_t i = 0; i < s->files.count; i++) {
    fstate *fst;
    dynamic_array_get(&s->files, i, &fst);
    free_fstate(fst);
  }

  dynamic_array_free(&s->files);

  free(s);
}
