#include "backend/llvm/ld_utils.hpp"
#include <cstdio>
#include <cstdlib>

extern "C" {
#include "utils.h"
}

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <vector>

void ld_link(const char *output_file,
             const std::vector<const char *> &obj_files) {
  std::vector<const char *> args;

  args.push_back("clang");
  args.push_back("-fuse-ld=lld");

  args.push_back("-o");
  args.push_back(output_file);

  for (const char *obj : obj_files) {
    args.push_back(obj);
  }

  args.push_back(nullptr);

  pid_t pid = fork();
  if (pid < 0) {
    scu_perror((char *)"fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    execvp("clang", const_cast<char *const *>(args.data()));
    perror("execvp clang");
    _exit(1);
  }

  int status = 0;
  waitpid(pid, &status, 0);

  if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
    scu_perror((char *)"Linking failed\n");
    exit(1);
  }
}
