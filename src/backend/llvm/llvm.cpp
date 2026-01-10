#include "backend/llvm/llvm_irgen.hpp"

extern "C" {
#include "ast.h"
#include "backend/llvm/llvm.h"
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
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  bctx.context = new llvm::LLVMContext();

  std::string module_name = fst->extracted_filepath;
  bctx.module = new llvm::Module(module_name, *bctx.context);

  bctx.builder = new llvm::IRBuilder<>(*bctx.context);

  bctx.target_triple = cst->llvm_target_triple;

  bctx.module->setTargetTriple(llvm::Triple(bctx.target_triple));

  bctx.target_machine = nullptr;
}

void llvm_backend_compile(cstate *, fstate *fst) {
  for (size_t i = 0; i < fst->program_ast.instrs.count; i++) {
    instr_node instr;
    dynamic_array_get(&fst->program_ast.instrs, i, &instr);

    llvm_irgen_instr(bctx, &instr);
  }
  llvm_irgen_clear_symbol_table();
}

void llvm_backend_emit(cstate *cst, fstate *fst) {
  std::string error_str;
  std::error_code ec;
  llvm::raw_string_ostream error_stream(error_str);

  if (llvm::verifyModule(*bctx.module, &error_stream)) {
    error_stream.flush();
    scu_perror(const_cast<char *>("Module verification failed: %s\n"),
               error_str.c_str());
    return;
  }

  if (cst->options.emit_llvm) {
    std::string ir_filename = std::string(fst->extracted_filepath) + ".ll";
    llvm::raw_fd_ostream ir_file(ir_filename, ec, llvm::sys::fs::OF_None);

    if (!ec) {
      bctx.module->print(ir_file, nullptr);
      ir_file.close();
    } else {
      scu_pwarning(const_cast<char *>("Could not write IR file: %s\n"),
                   ec.message().c_str());
    }

    return;
  }

  std::string error;
  const llvm::Target *target =
      llvm::TargetRegistry::lookupTarget(bctx.target_triple, error);

  if (!target) {
    scu_perror(const_cast<char *>("Failed to look up target: %s\n"),
               error.c_str());
    return;
  }

  llvm::TargetOptions opt;
  llvm::Reloc::Model RM = llvm::Reloc::PIC_;

  bctx.target_machine =
      target->createTargetMachine(llvm::Triple(bctx.target_triple), "generic",
                                  "", opt, RM, llvm::CodeModel::Small);

  if (!bctx.target_machine) {
    scu_perror(const_cast<char *>("Failed to create target machine\n"));
    return;
  }

  bctx.module->setDataLayout(bctx.target_machine->createDataLayout());

  if (cst->options.emit_asm) {
    std::string asm_filename = std::string(fst->extracted_filepath) + ".s";
    llvm::raw_fd_ostream asm_dest(asm_filename, ec, llvm::sys::fs::OF_None);

    if (ec) {
      scu_pwarning(const_cast<char *>("Could not open asm file: %s\n"),
                   ec.message().c_str());
      return;
    }

    llvm::legacy::PassManager asm_pass;
    if (bctx.target_machine->addPassesToEmitFile(
            asm_pass, asm_dest, nullptr, llvm::CodeGenFileType::AssemblyFile)) {
      scu_pwarning(const_cast<char *>("TargetMachine can't emit assembly\n"));
    } else {
      asm_pass.run(*bctx.module);
      asm_dest.flush();
    }

    return;
  }

  std::string obj_filename = std::string(fst->extracted_filepath) + ".o";
  llvm::raw_fd_ostream dest(obj_filename, ec, llvm::sys::fs::OF_None);

  if (ec) {
    scu_perror(const_cast<char *>("Could not open output file: %s\n"),
               ec.message().c_str());
    return;
  }

  llvm::legacy::PassManager pass;

  if (bctx.target_machine->addPassesToEmitFile(
          pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
    scu_perror(const_cast<char *>("TargetMachine can't emit object file\n"));
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
