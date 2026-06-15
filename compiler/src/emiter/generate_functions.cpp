#include "emiter/emiter.hpp"
#include <iostream>
#include "debug.hpp"

llvm::Value* LLVMGenerator::generateFunction(IRFunction* f) {
    DEBUG_OUTPUT << "Generating function: " << f->name << "\n";
    auto func = emiterModule->getFunction(f->name);

    if (!func) {
        std::cerr << "Function not found: " << f->name << "\n";
        return nullptr;
    }

    auto oldNamedValues = namedValues;
    namedValues.clear();

    size_t idx = 0;
    for (auto& arg : func->args()) {
        namedValues[f->args[idx++]] = &arg;
    }

    if (f->body.size() == 0) {
        namedValues = oldNamedValues;
        return func;
    }

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(emiterContext, "entry", func);
    emiterBuilder.SetInsertPoint(entry);

    for (auto& instr : f->body) {
        generate(instr.get());
    }

    if (!entry->getTerminator()) {
        if (f->returnType == "_BI_Void") {
            auto it = typeInfos.find("_BI_Void");
            if (it == typeInfos.end() || !it->second) {
                std::cerr << "Missing TypeInfo for _BI_Void\n";
                return nullptr;
            }
            llvm::Value* typeInfoGV = it->second;

            auto typeInfoIt = structTypes.find("_BI_TypeInfo");
            if (typeInfoIt == structTypes.end()) {
                std::cerr << "Missing _BI_TypeInfo struct type\n";
                return nullptr;
            }
            llvm::StructType* typeInfoTy = typeInfoIt->second;

            llvm::Type* i8PtrTy = llvm::Type::getInt8PtrTy(emiterContext);

            llvm::Value* typeInfoPtr = emiterBuilder.CreateBitCast(typeInfoGV, typeInfoTy->getPointerTo());

            llvm::Value* vtablePtrPtr = emiterBuilder.CreateStructGEP(typeInfoTy, typeInfoPtr, 2);

            llvm::Value* vtablePtr = emiterBuilder.CreateLoad(i8PtrTy->getPointerTo(), vtablePtrPtr);

            auto voidVtIt = vTablePos.find("_BI_Void");
            if (voidVtIt == vTablePos.end()) {
                std::cerr << "Missing vtable for _BI_Void\n";
                return nullptr;
            }
            auto initIt = voidVtIt->second.find("_BI_Void_init");
            if (initIt == voidVtIt->second.end()) {
                std::cerr << "Missing init in _BI_Void vtable\n";
                return nullptr;
            }
            int idx = initIt->second;
            llvm::Value* idxVal = llvm::ConstantInt::get(llvm::Type::getInt32Ty(emiterContext), idx);

            llvm::Value* fnPtrPtr = emiterBuilder.CreateInBoundsGEP(i8PtrTy, vtablePtr, idxVal);

            llvm::Value* fnPtr = emiterBuilder.CreateLoad(i8PtrTy, fnPtrPtr);

            llvm::Function* callee = emiterModule->getFunction("_BI_Void_init");
            if (!callee) {
                std::cerr << "Missing _BI_Void_init\n";
                return nullptr;
            }

            llvm::FunctionType* fnType = callee->getFunctionType();
            llvm::Value* typedFn = emiterBuilder.CreateBitCast(fnPtr, fnType->getPointerTo());

            llvm::Type* voidTy = mapLLVMType("_BI_Void", false);
            llvm::Function* mallocFn = emiterModule->getFunction("_BI_malloc");
            if (!mallocFn) {
                std::cerr << "Missing _BI_malloc\n";
                return nullptr;
            }
            const llvm::DataLayout& dl = emiterModule->getDataLayout();
            uint64_t totalSize = dl.getTypeAllocSize(voidTy);
            llvm::Value* sizeVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(emiterContext), totalSize);
            llvm::Value* allocated = emiterBuilder.CreateCall(mallocFn, {sizeVal});
            llvm::Value* objPtr = emiterBuilder.CreateBitCast(allocated, voidTy->getPointerTo());
            llvm::Value* typeInfoGEP = emiterBuilder.CreateStructGEP(voidTy, objPtr, 0);
            emiterBuilder.CreateStore(typeInfoPtr, typeInfoGEP);

            llvm::Value* call = emiterBuilder.CreateCall(fnType, typedFn, {objPtr});

            emiterBuilder.CreateRet(call);
        } else {
            std::cerr << "Missing return in function: " << f->name << "\n";
        }
    }

    namedValues = oldNamedValues;
    namedValues[f] = func;

    return func;
}

llvm::Value* LLVMGenerator::generateCall(IRCall* c) {
    llvm::Function* callee = emiterModule->getFunction(c->funcName);
    if (!callee) {
        std::cerr << "Function not found: " << c->funcName << "\n";
        return nullptr;
    }

    std::vector<llvm::Value*> args;
    for (auto argNode : c->args) {
        auto nvIt = namedValues.find(argNode);
        if (nvIt == namedValues.end() || !nvIt->second) {
            std::cerr << "Missing arg value in call: " << c->funcName << "\n";
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
            std::cerr << "Missing IR for function: " << c->funcName << "\n";
            return nullptr;
        }
        auto funcIr = dynamic_cast<IRFunction*>(ftIt->second);

        if (!funcIr) {
            std::cerr << "Missing IR for function: " << c->funcName << "\n";
            return nullptr;
        }

        className = funcIr->className;
        isStatic = funcIr->isStatic;
    }

    llvm::FunctionType* fnType = callee->getFunctionType();
    std::string methodName = getMethodName(c->funcName);
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

    llvm::Value* call = emiterBuilder.CreateCall(fnType, typedFn, args);

    namedValues[c] = call;
    return call;
}
