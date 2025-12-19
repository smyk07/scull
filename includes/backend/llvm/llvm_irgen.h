#ifndef LLVM_IRGEN_H
#define LLVM_IRGEN_H

#include "ast.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Target/TargetMachine.h>

typedef struct llvm_backend_ctx {
  llvm::LLVMContext *context;
  llvm::Module *module;
  llvm::IRBuilder<> *builder;
  llvm::TargetMachine *target_machine;
  std::string target_triple;
} llvm_backend_ctx;

void llvm_irgen_clear_symbol_table();

void llvm_irgen_instr(llvm_backend_ctx &ctx, instr_node *instr);

#endif // !LLVM_IRGEN_H
