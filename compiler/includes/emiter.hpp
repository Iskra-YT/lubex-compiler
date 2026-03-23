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

        inline llvm::Type* mapLLVMType(const std::string& type, bool getPointer = true) {
            if (type == "i32") return llvm::Type::getInt32Ty(emiterContext);
            if (type == "double") return llvm::Type::getDoubleTy(emiterContext);
            if (type == "i1") return llvm::Type::getInt1Ty(emiterContext);
            if (type == "i64") return llvm::Type::getInt64Ty(emiterContext);
            if (type == "void*") return llvm::Type::getVoidTy(emiterContext)->getPointerTo();

            auto it = structTypes.find(type);
            llvm::StructType* st = nullptr;
            if (it != structTypes.end()) {
                st = it->second;
            } else {
                st = llvm::StructType::create(emiterContext, type);
                structTypes[type] = st;
            }

            if (getPointer) {
                return st->getPointerTo();
            }

            return st;
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
            emiterModule->setDataLayout("e-m:e-i64:64-f80:128-n8:16:32:64-S128");

            generateBuildInStruct("_BI_Number", {"double"});
            generateBuildInFunction("_BI_Number_init", "_BI_Number", {"_BI_Number", "double"});
            generateBuildInFunction("_BI_Number_add", "_BI_Number", {"_BI_Number", "_BI_Number"});
            generateBuildInFunction("_BI_Number_subtract", "_BI_Number", {"_BI_Number", "_BI_Number"});
            generateBuildInFunction("_BI_Number_multiply", "_BI_Number", {"_BI_Number", "_BI_Number"});
            generateBuildInFunction("_BI_Number_divide", "_BI_Number", {"_BI_Number", "_BI_Number"});

            generateBuildInStruct("_BI_Void", {});
            generateBuildInFunction("_BI_Void_init", "_BI_Void", {"_BI_Void"});

            generateBuildInStruct("_BI_Object", {});
            generateBuildInFunction("_BI_Object_init", "_BI_Object", {"_BI_Object"});   

            generateBuildInStruct("_BI_String", {"i8*", "i64"});
            generateBuildInFunction("_BI_String_init", "_BI_String", {"_BI_String", "i8*"});

            generateBuildInFunction("_BI_malloc", "void*", {"i64"});
        }

        std::vector<llvm::Value*> generate(std::vector<std::unique_ptr<IRValue>> lir);
};

std::string mangleName(Symbol* sym);
std::string mangleName(const std::string &name);

#endif //EMITER_LUBEX_HPP