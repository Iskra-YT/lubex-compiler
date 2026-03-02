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
    
    public:
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

    private:

        llvm::Value* generate(IRValue* node);

        inline llvm::Function* generateBuildInFunction(std::string name, std::string returnType, std::vector<std::string> argTypes) {
            std::vector<llvm::Type*> args;
            for (auto arg : argTypes) {
                args.push_back(mapLLVMType(arg));
            }

            llvm::FunctionType* funcType = llvm::FunctionType::get(mapLLVMType(returnType), args, false);
            llvm::Function* func = llvm::Function::Create(funcType, llvm::GlobalValue::LinkageTypes::ExternalLinkage, name, emiterModule.get());
            return func;
        }

        inline llvm::StructType* generateBuildInStruct(std::string name, std::vector<std::string> body) {
            std::vector<llvm::Type*> types;
            for (auto type : body) {
                types.push_back(mapLLVMType(type));
            }

            llvm::StructType* namedStruct = llvm::StructType::create(emiterContext, name);
            namedStruct->setBody(types);
            structTypes[name] = namedStruct;
            return namedStruct;
        }

    public:
        LLVMGenerator(const std::string& moduleName) : emiterBuilder(emiterContext) {
            emiterModule = std::make_unique<llvm::Module>(moduleName, emiterContext);

            generateBuildInStruct("_BI_Int", {"double"});
            generateBuildInFunction("_BI_Int_init", "_BI_Int", {"double"});
            generateBuildInFunction("_BI_Int_add", "_BI_Int", {"_BI_Int", "_BI_Int"});
            generateBuildInFunction("_BI_Int_subtract", "_BI_Int", {"_BI_Int", "_BI_Int"});
            generateBuildInFunction("_BI_Int_multiply", "_BI_Int", {"_BI_Int", "_BI_Int"});
            generateBuildInFunction("_BI_Int_divide", "_BI_Int", {"_BI_Int", "_BI_Int"});

            generateBuildInStruct("_BI_Void", {});
            generateBuildInFunction("_BI_Void_init", "_BI_Void", {});
            generateBuildInFunction("_BI_Void_add", "_BI_Void", {"_BI_Void", "_BI_Void"});
            generateBuildInFunction("_BI_Void_subtract", "_BI_Void", {"_BI_Void", "_BI_Void"});
            generateBuildInFunction("_BI_Void_multiply", "_BI_Void", {"_BI_Void", "_BI_Void"});
            generateBuildInFunction("_BI_Void_divide", "_BI_Void", {"_BI_Void", "_BI_Void"});

            generateBuildInStruct("_BI_Object", {});
            generateBuildInFunction("_BI_Object_init", "_BI_Object", {});
            generateBuildInFunction("_BI_Object_add", "_BI_Object", {"_BI_Object", "_BI_Object"});
            generateBuildInFunction("_BI_Object_subtract", "_BI_Object", {"_BI_Object", "_BI_Object"});
            generateBuildInFunction("_BI_Object_multiply", "_BI_Object", {"_BI_Object", "_BI_Object"});
            generateBuildInFunction("_BI_Object_divide", "_BI_Object", {"_BI_Object", "_BI_Object"});
        }

        std::vector<llvm::Value*> generate(std::vector<std::unique_ptr<IRValue>> lir);
};

std::string mangleName(const std::string &name);

#endif //EMITER_LUBEX_HPP