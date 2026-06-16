#include "emiter/emiter.hpp"
#include <iostream>

std::unordered_map<std::string, Hash128> typeIds;
std::unordered_map<std::string, std::unordered_map<std::string, int>> structValues;
std::unordered_map<std::string, RTTIRecord> globalTypeInfos;

llvm::Value* LLVMGenerator::generate(IRValue* node) {
    if (auto n = dynamic_cast<IRNumber*>(node)) return generateNumber(n);
    if (auto a = dynamic_cast<IRAlloca*>(node)) return generateAlloca(a);
    if (auto v = dynamic_cast<IRVariableRead*>(node)) return generateVariableRead(v);
    if (auto s = dynamic_cast<IRStore*>(node)) return generateStore(s);
    if (auto f = dynamic_cast<IRFunction*>(node)) return generateFunction(f);
    if (auto c = dynamic_cast<IRCall*>(node)) return generateCall(c);
    if (auto r = dynamic_cast<IRReturn*>(node)) return generateReturn(r);
    if (auto s = dynamic_cast<IRString*>(node)) return generateString(s);
    if (auto a = dynamic_cast<IRAccess*>(node)) return generateAccess(a);
    if (auto g = dynamic_cast<IRAllocaStruct*>(node)) return generateAllocaStruct(g);
    if (auto n = dynamic_cast<IRNull*>(node)) return generateNull(n);
    if (auto n = dynamic_cast<IRNullCoalescing*>(node)) return generateNullCoalescing(n);
    if (auto n = dynamic_cast<IRNullCheck*>(node)) return generateNullCheck(n);
    if (auto s = dynamic_cast<IRSafeAccess*>(node)) return generateSafeAccess(s);
    if (auto s = dynamic_cast<IRSafeCall*>(node)) return generateSafeCall(s);

    return nullptr;
}

std::vector<llvm::Value*> LLVMGenerator::generate(std::vector<std::unique_ptr<IRValue>> lir) {
    std::vector<llvm::Value*> res;
    for (auto& instr : lir) {
        if (auto f = dynamic_cast<IRFunction*>(instr.get())) {
            std::vector<llvm::Type*> argTypes;
            for (auto arg : f->args) {
                argTypes.push_back(mapLLVMType(arg->type));
            }

            llvm::FunctionType* funcType = llvm::FunctionType::get(mapLLVMType(f->returnType), argTypes, false);
            llvm::Function* func = llvm::Function::Create(funcType, llvm::GlobalValue::LinkageTypes::ExternalLinkage,
                                                          f->name, emiterModule.get());
            typeMethods[f->className].push_back(func);
            functionTable[func] = instr.get();
        } else if (auto c = dynamic_cast<IRStruct*>(instr.get())) {
            std::vector<llvm::Type*> body;
            int memberIdx = 0;

            if (!c->parentName.empty() && structValues.count(c->parentName)) {
                structValues[c->name] = structValues[c->parentName];
                memberIdx = structTypes[c->parentName]->getNumElements();
            } else {
                memberIdx = structTypes["_BI_Object"]->getNumElements();
            }

            for (auto& type : c->data) {
                auto member = dynamic_cast<IRMember*>(type.get());
                if (!member) continue;

                structValues[c->name][member->name] = memberIdx++;
                body.push_back(mapLLVMType(member->type));
            }

            std::vector<std::string> parents;

            if (!c->parentName.empty()) {
                parents.push_back(c->parentName);
            } else {
                parents.push_back("_BI_Object");
            }

            generateStruct(c->name, body, parents);
        }
    }

    for (auto& [key, _] : typeMethods) {
        buildVTable(key);
    }

    for (auto& instr : lir) {
        res.push_back(generate(instr.get()));
    }

    return res;
}
