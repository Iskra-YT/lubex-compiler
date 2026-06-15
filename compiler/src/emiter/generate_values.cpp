#include "emiter/emiter.hpp"
#include <iostream>
#include "debug.hpp"

llvm::Value* LLVMGenerator::generateNumber(IRNumber* n) {
    auto num = llvm::ConstantFP::get(mapLLVMType(n->type), n->number);
    namedValues[n] = num;
    return num;
}

llvm::Value* LLVMGenerator::generateAlloca(IRAlloca* a) {
    llvm::Type* type = mapLLVMType(a->type);
    llvm::Value* ptr = emiterBuilder.CreateAlloca(type);
    namedValues[a] = ptr;
    return ptr;
}

llvm::Value* LLVMGenerator::generateVariableRead(IRVariableRead* v) {
    for (auto& [key, val] : namedValues) {
        if (key->name == v->name) {
            if (dynamic_cast<IRArg*>(key)) {
                namedValues[v] = val;
                return val;
            }

            llvm::Type* ty = mapLLVMType(v->type);
            auto var = emiterBuilder.CreateLoad(ty, val);
            namedValues[v] = var;
            return var;
        }
    }

    std::cerr << "Variable not found: " << v->name << "\n";
    return nullptr;
}

llvm::Value* LLVMGenerator::generateStore(IRStore* s) {
    llvm::Value* ptr = namedValues[s->ptr];
    llvm::Value* val = namedValues[s->value];

    auto store = emiterBuilder.CreateStore(val, ptr);
    namedValues[s] = store;

    return store;
}

llvm::Value* LLVMGenerator::generateReturn(IRReturn* r) {
    llvm::Value* ret;
    llvm::Value* value = namedValues[r->value];
    ret = emiterBuilder.CreateRet(value);

    namedValues[r] = ret;
    return ret;
}

llvm::Value* LLVMGenerator::generateString(IRString* s) {
    std::string str = s->value;

    llvm::Constant* constStr = llvm::ConstantDataArray::getString(emiterContext, str, true);

    auto global = new llvm::GlobalVariable(
        *emiterModule,
        constStr->getType(),
        true,
        llvm::GlobalValue::PrivateLinkage,
        constStr
    );

    llvm::Value* zero = llvm::ConstantInt::get(
        llvm::Type::getInt32Ty(emiterContext), 0
    );

    llvm::Value* indices[] = { zero, zero };

    llvm::Value* ptr = emiterBuilder.CreateInBoundsGEP(
        global->getValueType(),
        global,
        indices
    );

    namedValues[s] = ptr;
    return ptr;
}

llvm::Value* LLVMGenerator::generateAccess(IRAccess* a) {
    llvm::Value* obj = namedValues[a->object];
    if (!obj) {
        std::cerr << "Object not found in access: " << a->object->name << "\n";
        return nullptr;
    }

    int idx = a->memberName;
    
    if (!obj->getType()->isPointerTy()) {
        std::cerr << "Object is not a pointer type\n";
        return nullptr;
    }
    auto svIt = structValues.find(a->object->type);
    if (svIt == structValues.end()) {
        std::cerr << "Missing struct values for type: " << a->object->type << "\n";
        return nullptr;
    }
    auto memberIt = svIt->second.find("$" + std::to_string(a->memberName));
    if (memberIt == svIt->second.end()) {
        std::cerr << "Missing member index for type: " << a->object->type << ", member: " << a->memberName << "\n";
        return nullptr;
    }
    auto* gep = emiterBuilder.CreateStructGEP(mapLLVMType(a->object->type, false), obj, memberIt->second);
    namedValues[a] = gep;
    return gep;
}

llvm::Value* LLVMGenerator::generateAllocaStruct(IRAllocaStruct* g) {
    llvm::Type* type = mapLLVMType(g->type, false);

    llvm::Function* callee = emiterModule->getFunction("_BI_malloc");        
    if (!callee) return nullptr;

    const llvm::DataLayout &dl = emiterModule->getDataLayout();
    uint64_t totalSize = dl.getTypeAllocSize(type);

    llvm::Value* sizeVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(emiterContext), totalSize);

    llvm::Value* allocated = emiterBuilder.CreateCall(callee, { sizeVal });
    llvm::Value* objPtr = emiterBuilder.CreateBitCast(allocated, type->getPointerTo());


    auto tiIt = typeInfos.find(g->type);
    if (tiIt == typeInfos.end() || !tiIt->second) {
        std::cerr << "Missing TypeInfo for: " << g->type << "\n";
        return nullptr;
    }
    llvm::Value* typeInfo = tiIt->second;

    auto typeInfoStructIt = structTypes.find("_BI_TypeInfo");
    if (typeInfoStructIt == structTypes.end()) {
        std::cerr << "Missing _BI_TypeInfo struct type\n";
        return nullptr;
    }
    llvm::Value* typeInfoPtr = emiterBuilder.CreateBitCast(typeInfo, typeInfoStructIt->second->getPointerTo());
    llvm::Value* gep = emiterBuilder.CreateStructGEP(type, objPtr, 0);
    emiterBuilder.CreateStore(typeInfoPtr, gep);

    namedValues[g] = objPtr;
    return objPtr;
}
