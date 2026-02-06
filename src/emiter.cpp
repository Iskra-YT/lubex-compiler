#include "emiter.hpp"

std::unique_ptr<llvm::LLVMContext> emiterContext;
std::unique_ptr<llvm::Module> emiterModule;
std::unique_ptr<llvm::IRBuilder<>> emiterBuilder;

std::string mangleName(const std::string &name) {
    std::string mangledName = "_";
    size_t start = 0;
    size_t end = name.find('.');
    while (end != std::string::npos) {
        std::string part = name.substr(start, end - start);
        if (mangledName.back() == '_') {
            mangledName += "M" + std::to_string(part.length()) + part + "_";
        } else {
            mangledName += "C" + std::to_string(part.length()) + part + "_";
        }
        start = end + 1;
        end = name.find('.', start);
    }

    std::string part = name.substr(start);
    mangledName += "M" + std::to_string(part.length()) + part;
    return mangledName;
}

void setEmiter(std::string moduleName, std::string entryMethodName) {
    emiterContext = std::make_unique<llvm::LLVMContext>();
    emiterModule = std::make_unique<llvm::Module>(moduleName, *emiterContext);
    emiterBuilder = std::make_unique<llvm::IRBuilder<>>(*emiterContext);
}