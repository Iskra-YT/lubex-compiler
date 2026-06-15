#ifndef EMITER_LUBEX_HPP
#define EMITER_LUBEX_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include "LIR/lir.hpp"
#include "hash128.hpp"

extern std::unordered_map<std::string, Hash128> typeIds;
extern std::unordered_map<std::string, std::unordered_map<std::string, int>> structValues;
extern std::string mangleVisitor;

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

        llvm::Type* mapLLVMType(const std::string& type, bool getPointer = true);

    private:
        llvm::Value* generate(IRValue* node);
        llvm::Value* generateNumber(IRNumber* n);
        llvm::Value* generateAlloca(IRAlloca* a);
        llvm::Value* generateVariableRead(IRVariableRead* v);
        llvm::Value* generateStore(IRStore* s);
        llvm::Value* generateFunction(IRFunction* f);
        llvm::Value* generateCall(IRCall* c);
        llvm::Value* generateReturn(IRReturn* r);
        llvm::Value* generateString(IRString* s);
        llvm::Value* generateAccess(IRAccess* a);
        llvm::Value* generateAllocaStruct(IRAllocaStruct* g);

        llvm::StructType* generateStruct(const std::string& name, const std::vector<llvm::Type*>& types, const std::string parentName = "_BI_Object");
        llvm::StructType* generateStruct(const std::string& name, const std::vector<llvm::Type*>& types, const std::vector<std::string>& parents);
        llvm::StructType* generateStruct(const std::string name, const std::vector<std::string> body, const std::string parentName = "_BI_Object");

        llvm::Function* generateBuildInFunction(std::string name, std::string returnType, std::vector<std::string> argTypes, std::string className);
        llvm::StructType* generateBuildInStruct(std::string name, std::vector<std::string> body, const std::string parentName = "_BI_Object");

    public:
        LLVMGenerator(const std::string& moduleName);

        std::vector<llvm::Value*> generate(std::vector<std::unique_ptr<IRValue>> lir);
};

std::string mangleName(Symbol* sym);
std::string mangleName(const std::string &name);
std::string getMethodName(const std::string& fn);

std::unique_ptr<llvm::Module> generateRTTIModule(llvm::LLVMContext& ctx, const std::unordered_map<std::string, RTTIRecord>& globalTypeInfos, const std::string& moduleName);

#endif //EMITER_LUBEX_HPP
