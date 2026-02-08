#ifndef EMITER_LUBEX_HPP
#define EMITER_LUBEX_HPP

#include <memory>
#include <string>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

extern std::unique_ptr<llvm::LLVMContext> emiterContext;
extern std::unique_ptr<llvm::Module> emiterModule;
extern std::unique_ptr<llvm::IRBuilder<>> emiterBuilder;

std::string mangleName(const std::string &name);
void setEmiter(std::string moduleName);

#endif //EMITER_LUBEX_HPP