#ifndef EMITER_LUBEX_HPP
#define EMITER_LUBEX_HPP

#include <memory>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

extern std::unique_ptr<llvm::LLVMContext> emiterContext;
extern std::unique_ptr<llvm::Module> emiterModule;
extern std::unique_ptr<llvm::IRBuilder<>> emiterBuilder;

void setEmiter(std::string projectName);

#endif //EMITER_LUBEX_HPP