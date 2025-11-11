#include "emiter.hpp"

std::unique_ptr<llvm::LLVMContext> emiterContext;
std::unique_ptr<llvm::Module> emiterModule;
std::unique_ptr<llvm::IRBuilder<>> emiterBuilder;

void setEmiter(std::string projectName) {
    emiterContext = std::make_unique<llvm::LLVMContext>();
    emiterModule = std::make_unique<llvm::Module>(projectName, *emiterContext);
    emiterBuilder = std::make_unique<llvm::IRBuilder<>>(*emiterContext);
}