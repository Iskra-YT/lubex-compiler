#include "emiter.hpp"

std::unique_ptr<llvm::LLVMContext> emiterContext;
std::unique_ptr<llvm::Module> emiterModule;
std::unique_ptr<llvm::IRBuilder<>> emiterBuilder;

std::string mangleName(const std::string &name) {
    std::vector<std::string> parts;
    size_t start = 0;
    size_t end = name.find('.');
    
    while (end != std::string::npos) {
        parts.push_back(name.substr(start, end - start));
        start = end + 1;
        end = name.find('.', start);
    }

    parts.push_back(name.substr(start));

    std::string mangledName;
    if (parts.size() > 0) mangledName += "_M" + std::to_string(parts[0].length()) + parts[0];
    if (parts.size() > 1) mangledName += "_C" + std::to_string(parts[1].length()) + parts[1];
    if (parts.size() > 2) mangledName += "_F" + std::to_string(parts[2].length()) + parts[2];
    if (parts.size() > 3) mangledName += "_" + std::to_string(parts[3].length()) + parts[3];

    return mangledName;
}

void setEmiter(std::string moduleName) {
    emiterContext = std::make_unique<llvm::LLVMContext>();
    emiterModule = std::make_unique<llvm::Module>(moduleName, *emiterContext);
    emiterBuilder = std::make_unique<llvm::IRBuilder<>>(*emiterContext);
}