#include "backend/llvm/llvm_irgen.h"

extern "C" {
#include "ast.h"
#include "backend/backend.h"
#include "cstate.h"
#include "ds/dynamic_array.h"
#include "fstate.h"
#include "utils.h"
}

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>

#include <cstddef>

typedef struct llvm_backend_ctx llvm_backend_ctx;

static llvm_backend_ctx bctx;

extern "C" {
void llvm_backend_init(cstate *cst, fstate *fst) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmParser();
  llvm::InitializeNativeTargetAsmPrinter();

  bctx.context = new llvm::LLVMContext();

  std::string module_name = fst->extracted_filepath;
  bctx.module = new llvm::Module(module_name, *bctx.context);

  bctx.builder = new llvm::IRBuilder<>(*bctx.context);

  switch (cst->target) {
  case TARGET_LLVM_X86_64:
    bctx.target_triple = "x86_64-unknown-linux-gnu";
    break;
  case TARGET_LLVM_AARCH64:
    bctx.target_triple = "aarch64-unknown-linux-gnu";
    break;
  case TARGET_LLVM_RISCV64:
    bctx.target_triple = "riscv64-unknown-linux-gnu";
    break;
  default:
    bctx.target_triple = llvm::sys::getDefaultTargetTriple();
    break;
  }

  bctx.module->setTargetTriple(llvm::Triple(bctx.target_triple));

  bctx.target_machine = nullptr;
}

void llvm_backend_compile(cstate *, fstate *fst) {
  for (size_t i = 0; i < fst->program->instrs.count; i++) {
    instr_node instr;
    dynamic_array_get(&fst->program->instrs, i, &instr);

    llvm_irgen_instr(bctx, &instr);
  }
  llvm_irgen_clear_symbol_table();
}

void llvm_backend_emit(cstate *, fstate *fst) {
  std::string error_str;
  llvm::raw_string_ostream error_stream(error_str);

  if (llvm::verifyModule(*bctx.module, &error_stream)) {
    error_stream.flush();
    scu_perror(NULL, const_cast<char *>("Module verification failed: %s\n"),
               error_str.c_str());
    return;
  }

  // TEMPORARY: print IR to .ll file
  std::string ir_filename = std::string(fst->extracted_filepath) + ".ll";
  std::error_code ec;
  llvm::raw_fd_ostream ir_file(ir_filename, ec, llvm::sys::fs::OF_None);

  if (!ec) {
    bctx.module->print(ir_file, nullptr);
    ir_file.close();
  } else {
    scu_pwarning(const_cast<char *>("Could not write IR file: %s\n"),
                 ec.message().c_str());
  }

  std::string error;
  const llvm::Target *target =
      llvm::TargetRegistry::lookupTarget(bctx.target_triple, error);

  if (!target) {
    scu_perror(NULL, const_cast<char *>("Failed to look up target: %s\n"),
               error.c_str());
    return;
  }

  llvm::TargetOptions opt;
  llvm::Reloc::Model RM = llvm::Reloc::PIC_;

  bctx.target_machine =
      target->createTargetMachine(llvm::Triple(bctx.target_triple), "generic",
                                  "", opt, RM, llvm::CodeModel::Small);

  if (!bctx.target_machine) {
    scu_perror(NULL, const_cast<char *>("Failed to create target machine\n"));
    return;
  }

  bctx.module->setDataLayout(bctx.target_machine->createDataLayout());

  std::string obj_filename = std::string(fst->extracted_filepath) + ".o";

  llvm::raw_fd_ostream dest(obj_filename, ec, llvm::sys::fs::OF_None);

  if (ec) {
    scu_perror(NULL, const_cast<char *>("Could not open output file: %s\n"),
               ec.message().c_str());
    return;
  }

  llvm::legacy::PassManager pass;

  if (bctx.target_machine->addPassesToEmitFile(
          pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
    scu_perror(NULL,
               const_cast<char *>("TargetMachine can't emit object file\n"));
    return;
  }

  pass.run(*bctx.module);
  dest.flush();
}

void llvm_backend_cleanup(cstate *, fstate *) {
  delete bctx.builder;
  delete bctx.module;
  delete bctx.context;

  if (bctx.target_machine) {
    delete bctx.target_machine;
  }

  bctx.builder = nullptr;
  bctx.module = nullptr;
  bctx.context = nullptr;
  bctx.target_machine = nullptr;
}
}
