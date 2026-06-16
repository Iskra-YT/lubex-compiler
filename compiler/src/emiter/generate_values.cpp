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
    llvm::Value* found = nullptr;
    llvm::Value* allocaVal = nullptr;

    for (auto& [key, val] : namedValues) {
        if (key->name == v->name) {
            if (dynamic_cast<IRArg*>(key)) {
                namedValues[v] = val;
                return val;
            }

            found = val;
            if (dynamic_cast<IRAlloca*>(key)) {
                allocaVal = val;
            }
        }
    }

    if (allocaVal) found = allocaVal;

    if (!found) {
        std::cerr << "Variable not found: " << v->name << "\n";
        return nullptr;
    }

    llvm::Type* ty = mapLLVMType(v->type);
    auto var = emiterBuilder.CreateLoad(ty, found);
    namedValues[v] = var;
    return var;
}

llvm::Value* LLVMGenerator::generateStore(IRStore* s) {
    auto ptrIt = namedValues.find(s->ptr);
    auto valIt = namedValues.find(s->value);
    if (ptrIt == namedValues.end() || valIt == namedValues.end()) {
        std::cerr << "Missing values in store\n";
        return nullptr;
    }
    llvm::Value* ptr = ptrIt->second;
    llvm::Value* val = valIt->second;

    auto store = emiterBuilder.CreateStore(val, ptr);
    namedValues[s] = store;

    return store;
}

llvm::Value* LLVMGenerator::generateReturn(IRReturn* r) {
    auto valIt = namedValues.find(r->value);
    if (valIt == namedValues.end()) {
        std::cerr << "Missing value in return\n";
        return nullptr;
    }
    llvm::Value* ret;
    llvm::Value* value = valIt->second;
    ret = emiterBuilder.CreateRet(value);

    namedValues[r] = ret;
    return ret;
}

llvm::Value* LLVMGenerator::generateString(IRString* s) {
    std::string str = s->value;

    llvm::Constant* constStr = llvm::ConstantDataArray::getString(emiterContext, str, true);

    auto global =
        new llvm::GlobalVariable(*emiterModule, constStr->getType(), true, llvm::GlobalValue::PrivateLinkage, constStr);

    llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(emiterContext), 0);

    llvm::Value* indices[] = {zero, zero};

    llvm::Value* ptr = emiterBuilder.CreateInBoundsGEP(global->getValueType(), global, indices);

    namedValues[s] = ptr;
    return ptr;
}

llvm::Value* LLVMGenerator::generateAccess(IRAccess* a) {
    auto objIt = namedValues.find(a->object);
    if (objIt == namedValues.end() || !objIt->second) {
        std::cerr << "Object not found in access: " << a->object->name << "\n";
        return nullptr;
    }
    llvm::Value* obj = objIt->second;

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

    const llvm::DataLayout& dl = emiterModule->getDataLayout();
    uint64_t totalSize = dl.getTypeAllocSize(type);

    llvm::Value* sizeVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(emiterContext), totalSize);

    llvm::Value* allocated = emiterBuilder.CreateCall(callee, {sizeVal});
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

llvm::Value* LLVMGenerator::generateNull(IRNull* n) {
    llvm::Value* nullPtr = llvm::ConstantPointerNull::get(llvm::Type::getInt8PtrTy(emiterContext));
    namedValues[n] = nullPtr;
    return nullPtr;
}

llvm::Value* LLVMGenerator::generateNullCoalescing(IRNullCoalescing* nc) {
    auto leftIt = namedValues.find(nc->left);
    auto rightIt = namedValues.find(nc->right);
    if (leftIt == namedValues.end() || rightIt == namedValues.end() || !leftIt->second || !rightIt->second) {
        std::cerr << "Missing values for null coalescing\n";
        return nullptr;
    }
    llvm::Value* leftVal = leftIt->second;
    llvm::Value* rightVal = rightIt->second;

    llvm::Value* isNull = emiterBuilder.CreateICmpEQ(
        leftVal, llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(leftVal->getType())));

    llvm::Value* result = emiterBuilder.CreateSelect(isNull, rightVal, leftVal);
    namedValues[nc] = result;
    return result;
}

llvm::Value* LLVMGenerator::generateNullCheck(IRNullCheck* nc) {
    auto valIt = namedValues.find(nc->value);
    if (valIt == namedValues.end() || !valIt->second) {
        std::cerr << "Missing value for null check\n";
        return nullptr;
    }
    llvm::Value* val = valIt->second;

    llvm::Function* func = emiterBuilder.GetInsertBlock()->getParent();

    llvm::BasicBlock* panicBB = llvm::BasicBlock::Create(emiterContext, "panic", func);
    llvm::BasicBlock* contBB = llvm::BasicBlock::Create(emiterContext, "cont", func);

    llvm::Value* isNull =
        emiterBuilder.CreateICmpEQ(val, llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(val->getType())));
    emiterBuilder.CreateCondBr(isNull, panicBB, contBB);

    // Panic block
    emiterBuilder.SetInsertPoint(panicBB);
    llvm::Function* panicFn = emiterModule->getFunction("__R_panic");
    if (!panicFn) {
        std::cerr << "Missing __R_panic function\n";
        return nullptr;
    }
    std::string panicMsg = "Unexpected null value";
    llvm::Constant* msgConst = llvm::ConstantDataArray::getString(emiterContext, panicMsg, true);
    auto* global = new llvm::GlobalVariable(*emiterModule, msgConst->getType(), true, llvm::GlobalValue::PrivateLinkage,
                                            msgConst, ".panic_msg");
    llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(emiterContext), 0);
    llvm::Value* indices[] = {zero, zero};
    llvm::Value* msgPtr = emiterBuilder.CreateInBoundsGEP(global->getValueType(), global, indices);
    emiterBuilder.CreateCall(panicFn, {msgPtr});
    emiterBuilder.CreateUnreachable();

    // Continue block
    emiterBuilder.SetInsertPoint(contBB);
    namedValues[nc] = val;
    return val;
}

llvm::Value* LLVMGenerator::generateSafeAccess(IRSafeAccess* sa) {
    auto objIt = namedValues.find(sa->object);
    if (objIt == namedValues.end() || !objIt->second) {
        std::cerr << "Missing object for safe access\n";
        return nullptr;
    }
    llvm::Value* obj = objIt->second;

    llvm::Function* func = emiterBuilder.GetInsertBlock()->getParent();

    llvm::BasicBlock* nullBB = llvm::BasicBlock::Create(emiterContext, "safe_null", func);
    llvm::BasicBlock* accessBB = llvm::BasicBlock::Create(emiterContext, "safe_access", func);
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(emiterContext, "safe_merge", func);

    llvm::Value* isNull =
        emiterBuilder.CreateICmpEQ(obj, llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(obj->getType())));
    emiterBuilder.CreateCondBr(isNull, nullBB, accessBB);

    // Null block
    emiterBuilder.SetInsertPoint(nullBB);
    llvm::Value* nullResult = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(obj->getType()));
    emiterBuilder.CreateBr(mergeBB);

    // Access block
    emiterBuilder.SetInsertPoint(accessBB);
    auto svIt = structValues.find(sa->object->type);
    if (svIt == structValues.end()) {
        std::cerr << "Missing struct values for safe access type: " << sa->object->type << "\n";
        return nullptr;
    }
    auto memberIt = svIt->second.find("$" + std::to_string(sa->memberIndex));
    if (memberIt == svIt->second.end()) {
        std::cerr << "Missing member index for safe access\n";
        return nullptr;
    }
    llvm::Value* gep = emiterBuilder.CreateStructGEP(mapLLVMType(sa->object->type, false), obj, memberIt->second);
    llvm::Value* memberVal = emiterBuilder.CreateLoad(mapLLVMType(sa->type), gep);
    emiterBuilder.CreateBr(mergeBB);

    // Merge block
    emiterBuilder.SetInsertPoint(mergeBB);
    llvm::PHINode* phi = emiterBuilder.CreatePHI(obj->getType(), 2);
    phi->addIncoming(nullResult, nullBB);
    phi->addIncoming(memberVal, accessBB);

    namedValues[sa] = phi;
    return phi;
}

llvm::Value* LLVMGenerator::generateSafeCall(IRSafeCall* sc) {
    auto objIt = namedValues.find(sc->object);
    if (objIt == namedValues.end() || !objIt->second) {
        std::cerr << "Missing object for safe call\n";
        return nullptr;
    }
    llvm::Value* obj = objIt->second;

    llvm::Function* func = emiterBuilder.GetInsertBlock()->getParent();

    llvm::BasicBlock* nullBB = llvm::BasicBlock::Create(emiterContext, "safe_call_null", func);
    llvm::BasicBlock* callBB = llvm::BasicBlock::Create(emiterContext, "safe_call_call", func);
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(emiterContext, "safe_call_merge", func);

    llvm::Value* isNull =
        emiterBuilder.CreateICmpEQ(obj, llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(obj->getType())));
    emiterBuilder.CreateCondBr(isNull, nullBB, callBB);

    emiterBuilder.SetInsertPoint(nullBB);
    emiterBuilder.CreateBr(mergeBB);

    emiterBuilder.SetInsertPoint(callBB);

    llvm::Function* callee = emiterModule->getFunction(sc->funcName);
    if (!callee) {
        std::cerr << "Function not found in safe call: " << sc->funcName << "\n";
        return nullptr;
    }

    std::vector<llvm::Value*> args;
    for (auto argNode : sc->args) {
        auto nvIt = namedValues.find(argNode);
        if (nvIt == namedValues.end() || !nvIt->second) {
            std::cerr << "Missing arg value in safe call: " << sc->funcName << "\n";
            return nullptr;
        }
        args.push_back(nvIt->second);
    }

    std::string className;
    bool isStatic = false;

    if (builtinMethodClass.count(callee)) {
        className = builtinMethodClass[callee];
    } else {
        auto ftIt = functionTable.find(callee);
        if (ftIt == functionTable.end()) {
            std::cerr << "Missing IR for function: " << sc->funcName << "\n";
            return nullptr;
        }
        auto funcIr = dynamic_cast<IRFunction*>(ftIt->second);
        if (!funcIr) {
            std::cerr << "Missing IR for function: " << sc->funcName << "\n";
            return nullptr;
        }
        className = funcIr->className;
        isStatic = funcIr->isStatic;
    }

    llvm::FunctionType* fnType = callee->getFunctionType();
    std::string methodName = getMethodName(sc->funcName);
    auto clsVtIt = vTablePos.find(className);
    if (clsVtIt == vTablePos.end()) {
        std::cerr << "Missing vtable for class: " << className << "\n";
        return nullptr;
    }
    auto mthIt = clsVtIt->second.find(methodName);
    if (mthIt == clsVtIt->second.end()) {
        std::cerr << "Missing method in vtable: " << className << "." << methodName << "\n";
        return nullptr;
    }
    int idx = mthIt->second;

    llvm::Value* idxVal = llvm::ConstantInt::get(llvm::Type::getInt32Ty(emiterContext), idx);

    llvm::Type* i8PtrTy = llvm::Type::getInt8PtrTy(emiterContext);

    auto typeInfoIt = structTypes.find("_BI_TypeInfo");
    if (typeInfoIt == structTypes.end()) {
        std::cerr << "Missing _BI_TypeInfo struct type\n";
        return nullptr;
    }
    llvm::StructType* typeInfoTy = typeInfoIt->second;

    llvm::Value* vtablePtr = nullptr;
    if (isStatic) {
        auto tiIt = typeInfos.find(className);
        if (tiIt == typeInfos.end() || !tiIt->second) {
            std::cerr << "Missing TypeInfo for static class: " << className << "\n";
            return nullptr;
        }
        llvm::Value* typeInfoGV = tiIt->second;
        llvm::Value* typeInfoPtr = emiterBuilder.CreateBitCast(typeInfoGV, typeInfoTy->getPointerTo());
        llvm::Value* vtablePtrPtr = emiterBuilder.CreateStructGEP(typeInfoTy, typeInfoPtr, 2);
        vtablePtr = emiterBuilder.CreateLoad(i8PtrTy->getPointerTo(), vtablePtrPtr);
    } else {
        if (args.empty()) {
            std::cerr << "Missing this pointer for instance method call\n";
            return nullptr;
        }
        llvm::Value* thisPtr = args[0];
        auto* classTy = llvm::cast<llvm::StructType>(mapLLVMType(className, false));
        llvm::Value* typedThis = emiterBuilder.CreateBitCast(thisPtr, classTy->getPointerTo());
        llvm::Value* typeInfoPtrPtr = emiterBuilder.CreateStructGEP(classTy, typedThis, 0);
        llvm::Value* typeInfoPtr = emiterBuilder.CreateLoad(typeInfoTy->getPointerTo(), typeInfoPtrPtr);
        llvm::Value* vtablePtrPtr = emiterBuilder.CreateStructGEP(typeInfoTy, typeInfoPtr, 2);
        vtablePtr = emiterBuilder.CreateLoad(i8PtrTy->getPointerTo(), vtablePtrPtr);
    }

    llvm::Value* fnPtrPtr = emiterBuilder.CreateInBoundsGEP(i8PtrTy, vtablePtr, idxVal);
    llvm::Value* fnPtr = emiterBuilder.CreateLoad(i8PtrTy, fnPtrPtr);
    llvm::Value* typedFn = emiterBuilder.CreateBitCast(fnPtr, fnType->getPointerTo());
    llvm::Value* callVal = emiterBuilder.CreateCall(fnType, typedFn, args);

    emiterBuilder.CreateBr(mergeBB);

    // Merge block
    emiterBuilder.SetInsertPoint(mergeBB);

    llvm::Type* retTy = fnType->getReturnType();
    if (retTy->isVoidTy()) {
        namedValues[sc] = callVal;
    } else {
        llvm::Value* nullVal = llvm::Constant::getNullValue(retTy);
        llvm::PHINode* phi = emiterBuilder.CreatePHI(retTy, 2);
        phi->addIncoming(nullVal, nullBB);
        phi->addIncoming(callVal, callBB);
        namedValues[sc] = phi;
    }

    return namedValues[sc];
}
