#ifndef EMITER_LUBEX_HPP
#define EMITER_LUBEX_HPP

#include <memory>
#include <string>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include "LIR/lir.hpp"
#include "hash128.hpp"

extern std::unordered_map<std::string, Hash128> typeIds;

struct RTTIRecord {
    std::string name;
    std::string parent;
    Hash128 id;

    std::string vtableName;
    std::vector<std::string> vtableMethods;
};

extern std::unordered_map<std::string, RTTIRecord> globalTypeInfos;

class LLVMGenerator {
    public:
        llvm::LLVMContext emiterContext;
        llvm::IRBuilder<> emiterBuilder;
        std::unique_ptr<llvm::Module> emiterModule;

    private:
        std::unordered_map<IRValue*, llvm::Value*> namedValues;
        std::unordered_map<std::string, llvm::GlobalVariable*> typeInfos;
        std::unordered_map<std::string, std::vector<llvm::Function*>> typeMethods;
        std::unordered_map<std::string, std::vector<std::string>> classHierarchy;
        std::unordered_map<std::string, std::unordered_map<std::string, int>> vTablePos;
        std::unordered_map<std::string, std::unordered_map<std::string, llvm::Function*>> methodMap;
        std::unordered_map<llvm::Function*, IRValue*> functionTable;
        std::unordered_map<llvm::Function*, std::string> builtinMethodClass;
        std::unordered_map<std::string, std::vector<llvm::Function*>> vTables;

        llvm::GlobalVariable* getOrCreateTypeInfo(const std::string& name, const std::string& parentName = "");
        llvm::GlobalVariable* updateTypeInfoWithVTable(llvm::GlobalVariable* typeInfo, llvm::Constant* vtablePtr);
        void buildVTable(const std::string& cls);
    
    public:
        std::unordered_map<std::string, llvm::StructType*> structTypes;

        inline llvm::Type* mapLLVMType(const std::string& type, bool getPointer = true) {
            if (type == "i32") return llvm::Type::getInt32Ty(emiterContext);
            if (type == "double") return llvm::Type::getDoubleTy(emiterContext);
            if (type == "i1") return llvm::Type::getInt1Ty(emiterContext);
            if (type == "i64") return llvm::Type::getInt64Ty(emiterContext);
            if (type == "i64**") return llvm::Type::getInt64Ty(emiterContext)->getPointerTo()->getPointerTo();
            if (type == "i128") return llvm::Type::getInt128Ty(emiterContext);
            if (type == "void*") return llvm::Type::getVoidTy(emiterContext)->getPointerTo();
            if (type == "void**") return llvm::Type::getVoidTy(emiterContext)->getPointerTo()->getPointerTo();
            if (type == "i8**") return llvm::Type::getInt8Ty(emiterContext)->getPointerTo()->getPointerTo();

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

        inline llvm::StructType* generateStruct(const std::string& name, const std::vector<llvm::Type*>& types, const std::string parentName = "_BI_Object") {
            std::vector<llvm::Type*> finalTypes;
            if (!parentName.empty() && structTypes.count(parentName)) {
                classHierarchy[name].push_back(parentName);
                auto* parentStruct = structTypes[parentName];
            
                for (auto* ty : parentStruct->elements()) {
                    finalTypes.push_back(ty);
                }
            }

            for (auto* ty : types) {
                finalTypes.push_back(ty);
            }
        
            llvm::StructType* namedStruct = llvm::StructType::create(emiterContext, name);
            namedStruct->setBody(finalTypes);
            structTypes[name] = namedStruct;

            getOrCreateTypeInfo(name, parentName);
            return namedStruct;
        }

        inline llvm::StructType* generateStruct(const std::string& name, const std::vector<llvm::Type*>& types, const std::vector<std::string>& parents = {"_BI_Object"}) {
            std::vector<llvm::Type*> finalTypes;

            classHierarchy[name] = parents;

            for (auto& parentName : parents) {
                if (parentName.empty()) continue;
            
                auto it = structTypes.find(parentName);
                if (it == structTypes.end()) continue;
            
                auto* parentStruct = it->second;
            
                for (auto* ty : parentStruct->elements()) {
                    finalTypes.push_back(ty);
                }
            }
        
            for (auto* ty : types) {
                finalTypes.push_back(ty);
            }
        
            llvm::StructType* namedStruct = llvm::StructType::create(emiterContext, name);
            namedStruct->setBody(finalTypes);
            structTypes[name] = namedStruct;
    
            getOrCreateTypeInfo(name, parents.back());
        
            return namedStruct;
        }

        inline llvm::StructType* generateStruct(const std::string name, const std::vector<std::string> body, const std::string parentName = "_BI_Object") {
            std::vector<llvm::Type*> types;
            for (const auto& type : body) {
                types.push_back(mapLLVMType(type));
            }

            return generateStruct(name, types, parentName);
        }

        inline llvm::Function* generateBuildInFunction(std::string name, std::string returnType, std::vector<std::string> argTypes, std::string className) {
            std::vector<llvm::Type*> args;
            for (auto arg : argTypes) {
                args.push_back(mapLLVMType(arg));
            }

            llvm::FunctionType* funcType = llvm::FunctionType::get(mapLLVMType(returnType), args, false);
            llvm::Function* func = llvm::Function::Create(funcType, llvm::GlobalValue::LinkageTypes::ExternalLinkage, name, emiterModule.get());
            
            if (!className.empty()) {
                typeMethods[className].push_back(func);
                builtinMethodClass[func] = className;
            }

            return func;
        }

        inline llvm::StructType* generateBuildInStruct(std::string name, std::vector<std::string> body, const std::string parentName = "_BI_Object") {
            return generateStruct(name, body, parentName);
        }

    public:
        LLVMGenerator(const std::string& moduleName) : emiterBuilder(emiterContext) {
            emiterModule = std::make_unique<llvm::Module>(moduleName, emiterContext);
            emiterModule->setDataLayout("e-m:e-i64:64-f80:128-n8:16:32:64-S128");

            generateBuildInStruct("_BI_TypeInfo", {"i128", "_BI_TypeInfo", "i64**"}, "");

            generateBuildInStruct("_BI_Object", {"_BI_TypeInfo"});
            generateBuildInFunction("_BI_Object_init", "_BI_Object", {"_BI_Object"}, "_BI_Object");  
            generateBuildInFunction("_BI_Object_toString", "_BI_Object", {"_BI_Object"}, "_BI_String");  

            generateBuildInStruct("_BI_Number", {"double"});
            generateBuildInFunction("_BI_Number_init", "_BI_Number", {"_BI_Number", "double"}, "_BI_Number");
            generateBuildInFunction("_BI_Number_add", "_BI_Number", {"_BI_Number", "_BI_Number"}, "_BI_Number");
            generateBuildInFunction("_BI_Number_subtract", "_BI_Number", {"_BI_Number", "_BI_Number"}, "_BI_Number");
            generateBuildInFunction("_BI_Number_multiply", "_BI_Number", {"_BI_Number", "_BI_Number"}, "_BI_Number");
            generateBuildInFunction("_BI_Number_divide", "_BI_Number", {"_BI_Number", "_BI_Number"}, "_BI_Number");

            generateBuildInStruct("_BI_Void", {});
            generateBuildInFunction("_BI_Void_init", "_BI_Void", {"_BI_Void"}, "_BI_Void"); 

            generateBuildInStruct("_BI_String", {"i8*", "i64"});
            generateBuildInFunction("_BI_String_init", "_BI_String", {"_BI_String", "i8*"}, "_BI_String");

            generateBuildInFunction("_BI_malloc", "void*", {"i64"}, "");
        }

        std::vector<llvm::Value*> generate(std::vector<std::unique_ptr<IRValue>> lir);
};

std::string mangleName(Symbol* sym);
std::string mangleName(const std::string &name);

std::unique_ptr<llvm::Module> generateRTTIModule(llvm::LLVMContext& ctx, const std::unordered_map<std::string, RTTIRecord>& globalTypeInfos, const std::string& moduleName);

#endif //EMITER_LUBEX_HPP