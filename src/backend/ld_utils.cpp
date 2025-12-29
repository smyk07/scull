extern "C" {
#include "backend/ld_utils.h"
#include "utils.h"
}

#include <lld/Common/Driver.h>

LLD_HAS_DRIVER(elf)

extern "C" {
void ld_link(const char *output_file, const char *obj_file) {
  std::vector<const char *> args;

  args.push_back("ld.lld");
  args.push_back("-static");
  args.push_back("-o");
  args.push_back(output_file);

  args.push_back("-L/usr/lib");
  args.push_back("-L/usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1");

  args.push_back("/usr/lib/crt1.o");
  args.push_back("/usr/lib/crti.o");
  args.push_back(obj_file);

  args.push_back("--start-group");
  args.push_back("-lc");
  args.push_back("-lgcc");
  args.push_back("-lgcc_eh");
  args.push_back("--end-group");

  args.push_back("/usr/lib/crtn.o");

  lld::Result result =
      lld::lldMain(llvm::ArrayRef<const char *>(args), llvm::outs(),
                   llvm::errs(), {{lld::Gnu, &lld::elf::link}});

  if (result.retCode != 0) {
    scu_perror(const_cast<char *>("Linking failed\n"));
    exit(1);
  }
}
}
