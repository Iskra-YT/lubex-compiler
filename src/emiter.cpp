#include "emiter.hpp"

std::unique_ptr<llvm::LLVMContext> emiterContext;
std::unique_ptr<llvm::Module> emiterModule;
std::unique_ptr<llvm::IRBuilder<>> emiterBuilder;

void setEmiter(std::string moduleName) {
    emiterContext = std::make_unique<llvm::LLVMContext>();
    emiterModule = std::make_unique<llvm::Module>(moduleName, *emiterContext);
    emiterBuilder = std::make_unique<llvm::IRBuilder<>>(*emiterContext);
}