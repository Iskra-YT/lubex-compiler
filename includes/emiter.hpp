#ifndef EMITER_LUBEX_HPP
#define EMITER_LUBEX_HPP

#include <memory>
#include <string>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include "LIR/lir.hpp"

class LLVMGenerator {
    public:
        llvm::LLVMContext emiterContext;
        llvm::IRBuilder<> emiterBuilder;
        std::unique_ptr<llvm::Module> emiterModule;

    private:
        std::unordered_map<IRValue*, llvm::Value*> namedValues;
        std::unordered_map<std::string, llvm::StructType*> structTypes;
        
        inline llvm::Type* mapLLVMType(const std::string& type) {
            if (type == "i32") return llvm::Type::getInt32Ty(emiterContext);
            if (type == "double") return llvm::Type::getDoubleTy(emiterContext);
            if (type == "i1") return llvm::Type::getInt1Ty(emiterContext);

            auto it = structTypes.find(type);
            llvm::StructType* st = nullptr;
            if (it != structTypes.end()) {
                st = it->second;
            } else {
                st = llvm::StructType::create(emiterContext, type);
                structTypes[type] = st;
            }

            return st->getPointerTo();
        }

        llvm::Value* generate(IRValue* node);

    public:
        LLVMGenerator(const std::string& moduleName) : emiterBuilder(emiterContext) {
            emiterModule = std::make_unique<llvm::Module>(moduleName, emiterContext);
        }

        std::vector<llvm::Value*> generate(std::vector<std::unique_ptr<IRValue>> lir);
};

std::string mangleName(const std::string &name);

#endif //EMITER_LUBEX_HPP