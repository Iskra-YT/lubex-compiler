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
            auto* typeInfoGV = typeInfos["_BI_Void"];

            llvm::Type* i8PtrTy = llvm::Type::getInt8PtrTy(emiterContext);

            llvm::Value* typeInfoPtr = emiterBuilder.CreateBitCast(
                typeInfoGV,
                structTypes["_BI_TypeInfo"]->getPointerTo()
            );
            
            llvm::Value* vtablePtrPtr = emiterBuilder.CreateStructGEP(
                structTypes["_BI_TypeInfo"],
                typeInfoPtr,
                2
            );
            
            llvm::Value* vtablePtr = emiterBuilder.CreateLoad(
                i8PtrTy->getPointerTo(),
                vtablePtrPtr
            );
            
            int idx = vTablePos["_BI_Void"]["init"];
            llvm::Value* idxVal = llvm::ConstantInt::get(
                llvm::Type::getInt32Ty(emiterContext),
                idx
            );
            
            llvm::Value* fnPtrPtr = emiterBuilder.CreateInBoundsGEP(
                i8PtrTy,
                vtablePtr,
                idxVal
            );
            
            llvm::Value* fnPtr = emiterBuilder.CreateLoad(
                i8PtrTy,
                fnPtrPtr
            );
            
            llvm::Function* callee = emiterModule->getFunction("_BI_Void_init");  
            if (!callee) {
                std::cerr << "Missing _BI_Void_init\n";
                return nullptr;
            }

            llvm::FunctionType* fnType = callee->getFunctionType();
            llvm::Value* typedFn = emiterBuilder.CreateBitCast(
                fnPtr,
                fnType->getPointerTo()
            );
            
            llvm::Value* call = emiterBuilder.CreateCall(
                fnType,
                typedFn,
                {}
            );
            
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
        llvm::Value* argVal = namedValues[argNode];
    
        if (!argVal) {
            std::cerr << "Missing arg value in call: " << c->funcName << "\n";
            return nullptr;
        }
    
        args.push_back(argVal);
    }

    std::string className;
    bool isStatic = false;

    if (builtinMethodClass.count(callee)) {
        className = builtinMethodClass[callee];
    } else {
        auto funcIr = dynamic_cast<IRFunction*>(functionTable[callee]);
    
        if (!funcIr) {
            std::cerr << "Missing IR for function\n";
            return nullptr;
        }
    
        className = funcIr->className;
        isStatic = funcIr->isStatic;
    }

    llvm::FunctionType* fnType = callee->getFunctionType();
    std::string methodName = getMethodName(c->funcName);
    int idx = vTablePos[className][methodName];

    llvm::Value* idxVal = llvm::ConstantInt::get(
        llvm::Type::getInt32Ty(emiterContext),
        idx
    );
    
    llvm::Type* i8PtrTy = llvm::Type::getInt8PtrTy(emiterContext);
    llvm::Value* vtablePtr = nullptr;
    if (isStatic) {
        auto* typeInfoGV = typeInfos[className];
    
        llvm::Value* typeInfoPtr = emiterBuilder.CreateBitCast(
            typeInfoGV,
            structTypes["_BI_TypeInfo"]->getPointerTo()
        );
        
        llvm::Value* vtablePtrPtr = emiterBuilder.CreateStructGEP(
            structTypes["_BI_TypeInfo"],
            typeInfoPtr,
            2
        );
        
        vtablePtr = emiterBuilder.CreateLoad(
            i8PtrTy->getPointerTo(),
            vtablePtrPtr
        );
    } else {
        llvm::Value* thisPtr = args[0];
        auto* classTy = llvm::cast<llvm::StructType>(mapLLVMType(className, false));

        llvm::Value* typedThis = emiterBuilder.CreateBitCast(
            thisPtr,
            classTy->getPointerTo()
        );

        llvm::Value* typeInfoPtrPtr = emiterBuilder.CreateStructGEP(
            classTy,
            typedThis,
            0
        );
        
        llvm::Value* typeInfoPtr = emiterBuilder.CreateLoad(
            structTypes["_BI_TypeInfo"]->getPointerTo(),
            typeInfoPtrPtr
        );
        
        llvm::Value* vtablePtrPtr = emiterBuilder.CreateStructGEP(
            structTypes["_BI_TypeInfo"],
            typeInfoPtr,
            2
        );
        
        vtablePtr = emiterBuilder.CreateLoad(
            i8PtrTy->getPointerTo(),
            vtablePtrPtr
        );
    }

    llvm::Value* fnPtrPtr = emiterBuilder.CreateInBoundsGEP(
        i8PtrTy,
        vtablePtr,
        idxVal
    );
    
    llvm::Value* fnPtr = emiterBuilder.CreateLoad(
        i8PtrTy,
        fnPtrPtr
    );
    
    llvm::Value* typedFn = emiterBuilder.CreateBitCast(
        fnPtr,
        fnType->getPointerTo()
    );
    
    llvm::Value* call = emiterBuilder.CreateCall(
        fnType,
        typedFn,
        args
    );
    
    namedValues[c] = call;
    return call;
}
