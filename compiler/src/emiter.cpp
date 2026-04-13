#include "emiter.hpp"

std::unordered_map<std::string, Hash128> typeIds;
std::unordered_map<IRValue*, std::unordered_map<std::string, int>> structValues;

extern std::string mangleVisitor;

std::string mangleName(const std::string &name) {
    if (mangleVisitor != "") {
        return mangleVisitor;
    }

    std::vector<std::string> parts;
    size_t start = 0;
    size_t end = name.find('.');
    
    while (end != std::string::npos) {
        parts.push_back(name.substr(start, end - start));
        start = end + 1;
        end = name.find('.', start);
    }

    parts.push_back(name.substr(start));

    std::string mangledName;
    if (parts.size() > 0) mangledName += "_M" + std::to_string(parts[0].length()) + parts[0];
    if (parts.size() > 1) mangledName += "_C" + std::to_string(parts[1].length()) + parts[1];
    if (parts.size() > 2) mangledName += "_F" + std::to_string(parts[2].length()) + parts[2];
    if (parts.size() > 3) mangledName += "_" + std::to_string(parts[3].length()) + parts[3];

    return mangledName;
}

std::string mangleName(Symbol* sym) {
    if (!sym->forcedMangle.empty()) {
        return sym->forcedMangle;
    }

    return mangleName(sym->mangledName);
}

std::string getMethodName(std::string fn) {
    auto pos = fn.find_last_of('_');
    if (pos == std::string::npos) return fn;
    return fn.substr(pos + 1);
}

llvm::GlobalVariable* LLVMGenerator::updateTypeInfoWithVTable(llvm::GlobalVariable* typeInfo, llvm::Constant* vtablePtr) {
    auto* ty = structTypes["_BI_TypeInfo"];

    llvm::Constant* oldInit = typeInfo->getInitializer();
    auto* oldStruct = llvm::dyn_cast<llvm::ConstantStruct>(oldInit);

    if (!oldStruct) {
        std::cerr << "Invalid TypeInfo initializer\n";
        return typeInfo;
    }

    std::vector<llvm::Constant*> newFields;

    for (unsigned i = 0; i < 2; i++) {
        newFields.push_back(oldStruct->getOperand(i));
    }

    newFields.push_back(vtablePtr);

    llvm::Constant* newInit = llvm::ConstantStruct::get(ty, newFields);

    typeInfo->setInitializer(newInit);
    return typeInfo;
}

llvm::GlobalVariable* LLVMGenerator::getOrCreateTypeInfo(const std::string& name, const std::string& parentName) {
    if (typeInfos.count(name)) return typeInfos[name];

    auto* ty = structTypes["_BI_TypeInfo"];

    Hash128 id;
    if (typeIds.count(name)) id = typeIds[name];
    else id = hash128(name);

    llvm::Constant* parent = llvm::ConstantPointerNull::get(ty->getPointerTo());

    if (!parentName.empty() && typeInfos.count(parentName)) {
        parent = typeInfos[parentName];
    }

    llvm::Constant* vtable = llvm::ConstantPointerNull::get(llvm::Type::getInt64PtrTy(emiterContext)->getPointerTo()); // TODO: Change null into vTable ptr

    llvm::Constant* idConst = llvm::ConstantInt::get(llvm::Type::getInt128Ty(emiterContext), toAPInt(id));
    llvm::Constant* init = llvm::ConstantStruct::get(ty, {idConst, parent, vtable});

    auto* gv = new llvm::GlobalVariable(*emiterModule, ty, true, llvm::GlobalValue::LinkageTypes::InternalLinkage, init, "_T" + name);
    typeInfos[name] = gv;
    return gv;
}

llvm::Value* LLVMGenerator::generate(IRValue* node) {
    if (auto n = dynamic_cast<IRNumber*>(node)) {
        auto num = llvm::ConstantFP::get(mapLLVMType(n->type), n->number);
        namedValues[n] = num;
        return num;
    } else if (auto a = dynamic_cast<IRAlloca*>(node)) {
        llvm::Type* type = mapLLVMType(a->type);

        llvm::Function* callee = emiterModule->getFunction("_BI_malloc");
        if (!callee) {
            std::cerr << "Function not found: _BI_malloc\n";
            return nullptr;
        }

        llvm::DataLayout DL(emiterModule.get());
        uint64_t sizeBytes = DL.getTypeAllocSize(type);
        llvm::Value* sizeVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(emiterContext), sizeBytes);

        llvm::Value* mallocCall = emiterBuilder.CreateCall(callee, {sizeVal});
        llvm::Value* typedPtr = emiterBuilder.CreateBitCast(mallocCall, type->getPointerTo());

        namedValues[a] = typedPtr;
        return typedPtr;
    } else if (auto v = dynamic_cast<IRVariableRead*>(node)) {
        for (auto& [key, val] : namedValues) {
            if (key->name == v->name) {
                auto var = emiterBuilder.CreateLoad(mapLLVMType(v->type), val);
                namedValues[v] = var;
                return var;
            }
        }

        std::cerr << "Variable not found: " << v->name << "\n";
        return nullptr;
    } else if (auto s = dynamic_cast<IRStore*>(node)) {
        llvm::Value* ptr = namedValues[s->ptr];
        llvm::Value* val = namedValues[s->value];

        auto store = emiterBuilder.CreateStore(val, ptr);
        namedValues[s] = store;

        return store;
    } else if (auto f = dynamic_cast<IRFunction*>(node)) {
        DEBUG_OUTPUT << "Generating function: " << f->name << "\n";
        auto func = emiterModule->getFunction(f->name);

        if (!func) {
            std::cerr << "Function not found: " << f->name << "\n";
            return nullptr;
        }

        size_t idx = 0;
        for (auto& arg : func->args()) {
            namedValues[f->args[idx++]] = &arg;
        }

        if (f->body.size() == 0) {
            return func;
        }

        llvm::BasicBlock* entry = llvm::BasicBlock::Create(emiterContext, "entry", func);
        emiterBuilder.SetInsertPoint(entry);

        for (auto& instr : f->body) {
            generate(instr.get());
        }

        namedValues[f] = func;

        return func;
    } else if (auto c = dynamic_cast<IRCall*>(node)) {
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
            isStatic = false; 
        } else {
            auto funcIr = dynamic_cast<IRFunction*>(functionTable[callee]);
        
            if (!funcIr) {
                std::cerr << "Missing IR for function\n";
                return nullptr;
            }
        
            className = funcIr->className;
            isStatic = funcIr->isStatic;
        }

        llvm::Value* call = nullptr;
        if (isStatic) {
            auto* typeInfoGV = typeInfos[className];
            llvm::Value* typeInfoPtr = emiterBuilder.CreateBitCast(typeInfoGV, structTypes["_BI_TypeInfo"]->getPointerTo());

            llvm::Value* vtablePtrPtr = emiterBuilder.CreateStructGEP(structTypes["_BI_TypeInfo"], typeInfoPtr, 2);
            llvm::Value* vtablePtr = emiterBuilder.CreateLoad(llvm::Type::getInt8PtrTy(emiterContext)->getPointerTo(), vtablePtrPtr);

            std::string methodName = getMethodName(c->funcName);
            int idx = vTablePos[className][methodName];

            llvm::Value* idxVal = llvm::ConstantInt::get(llvm::Type::getInt32Ty(emiterContext), idx);

            llvm::Value* fnPtrPtr = emiterBuilder.CreateGEP(llvm::Type::getInt8PtrTy(emiterContext), vtablePtr, idxVal);
            llvm::Value* fnPtr = emiterBuilder.CreateLoad(llvm::Type::getInt8PtrTy(emiterContext), fnPtrPtr);

            llvm::FunctionType* fnType = callee->getFunctionType();

            llvm::Value* typedFn = emiterBuilder.CreateBitCast(fnPtr, fnType->getPointerTo());
            call = emiterBuilder.CreateCall(fnType, typedFn, args);
        } else {
            llvm::Value* thisPtr = args[0];

            llvm::Value* typeInfoPtrPtr = emiterBuilder.CreateStructGEP(mapLLVMType(className, false), thisPtr, 0);
            llvm::Value* typeInfoPtr = emiterBuilder.CreateLoad(structTypes["_BI_TypeInfo"]->getPointerTo(), typeInfoPtrPtr);

            llvm::Value* vtablePtrPtr = emiterBuilder.CreateStructGEP(structTypes["_BI_TypeInfo"], typeInfoPtr, 2);

            llvm::Value* vtablePtr = emiterBuilder.CreateLoad(llvm::Type::getInt8PtrTy(emiterContext)->getPointerTo(), vtablePtrPtr);

            std::string methodName = getMethodName(c->funcName);
            int idx = vTablePos[className][methodName];
            llvm::Value* idxVal = llvm::ConstantInt::get(llvm::Type::getInt32Ty(emiterContext), idx);

            llvm::Value* fnPtrPtr = emiterBuilder.CreateGEP(llvm::Type::getInt8PtrTy(emiterContext), vtablePtr, idxVal);
            llvm::Value* fnPtr = emiterBuilder.CreateLoad(llvm::Type::getInt8PtrTy(emiterContext), fnPtrPtr);

            llvm::FunctionType* fnType = callee->getFunctionType();
            llvm::Value* typedFn = emiterBuilder.CreateBitCast(fnPtr, fnType->getPointerTo());
            call = emiterBuilder.CreateCall(fnType, typedFn, args);
        }

        namedValues[c] = call;
        return call;
    } else if (auto r = dynamic_cast<IRReturn*>(node)) {
        llvm::Value* ret;
        llvm::Value* value = namedValues[r->value];
        ret = emiterBuilder.CreateRet(value);

        namedValues[r] = ret;
        return ret;
    } else if (auto s = dynamic_cast<IRString*>(node)) {
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
    } else if (auto a = dynamic_cast<IRAccess*>(node)) {
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
        auto* gep = emiterBuilder.CreateStructGEP(mapLLVMType(a->object->type, false), obj, idx + 1);
        namedValues[a] = gep;
        return gep;
    } else if (auto g = dynamic_cast<IRAllocaStruct*>(node)) {
        llvm::Type* type = mapLLVMType(g->type, false);

        llvm::Function* callee = emiterModule->getFunction("_BI_malloc");        if (!callee) return nullptr;

        const llvm::DataLayout &dl = emiterModule->getDataLayout();
        uint64_t size = dl.getTypeAllocSize(type);
        uint64_t typeInfoSize = dl.getPointerSize();
        uint64_t totalSize = size + typeInfoSize;

        llvm::Value* sizeVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(emiterContext), totalSize);

        llvm::Value* allocated = emiterBuilder.CreateCall(callee, { sizeVal });
        llvm::Value* objPtr = emiterBuilder.CreateBitCast(allocated, type->getPointerTo());


        llvm::Value* typeInfo = typeInfos[g->type];
        if (!typeInfo) {
            std::cerr << "Missing TypeInfo for: " << g->type << "\n";
            return nullptr;
        }

        llvm::Value* typeInfoPtr = emiterBuilder.CreateBitCast(typeInfo, structTypes["_BI_TypeInfo"]->getPointerTo());
        llvm::Value* gep = emiterBuilder.CreateStructGEP(type, objPtr, 0);
        emiterBuilder.CreateStore(typeInfoPtr, gep);

        namedValues[g] = objPtr;
        return objPtr;
    }

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
            llvm::Function* func = llvm::Function::Create(funcType, llvm::GlobalValue::LinkageTypes::ExternalLinkage, f->name, emiterModule.get());
            typeMethods[f->className].push_back(func);
            functionTable[func] = instr.get();
        } else if (auto c = dynamic_cast<IRStruct*>(instr.get())) {
            std::vector<llvm::Type*> body;
            int memberIdx = 0;
            for (auto& type : c->data) {
                auto member = dynamic_cast<IRMember*>(type.get());
                if (!member) continue;

                structValues[instr.get()][member->name] = memberIdx++;
                body.push_back(mapLLVMType(member->type));
            }

            llvm::StructType* namedStruct = generateStruct(c->name, body);
        }
    }

    for (auto& [key, methods] : typeMethods) {
        
        std::string parent = classHierarchy[key].empty() ? "" : classHierarchy[key][0];
        std::vector<llvm::Function*> finalMethods;
        
        std::unordered_map<std::string, int> indexMap;
        if (!parent.empty()) {
            for (auto* fn : typeMethods[parent]) {
                std::string name = getMethodName(fn->getName().str());
            
                indexMap[name] = finalMethods.size();
                finalMethods.push_back(fn);
            }
        }

        for (auto* fn : methods) {
            std::string name = getMethodName(fn->getName().str());
        
            if (indexMap.count(name)) {
                int idx = indexMap[name];
                finalMethods[idx] = fn;
                vTablePos[key][name] = idx;
            } else {
                vTablePos[key][name] = finalMethods.size();
                indexMap[name] = finalMethods.size();
                finalMethods.push_back(fn);
            }
        }
    
        std::vector<llvm::Constant*> entries;
        entries.reserve(finalMethods.size());
    
        for (auto* fn : finalMethods) {
            entries.push_back(
                llvm::ConstantExpr::getBitCast(
                    fn,
                    llvm::Type::getInt8PtrTy(emiterContext)
                )
            );
        }
    
        auto* vtableType = llvm::ArrayType::get(llvm::Type::getInt8PtrTy(emiterContext), entries.size());
        llvm::Constant* vTable = llvm::ConstantArray::get(vtableType, entries);
        auto* vTableGlobal = new llvm::GlobalVariable(*emiterModule, vtableType, true, llvm::GlobalValue::InternalLinkage, vTable, "_VT" + key);
        auto* vtablePtr = llvm::ConstantExpr::getBitCast(vTableGlobal, llvm::Type::getInt8PtrTy(emiterContext)->getPointerTo());
        typeInfos[key] = updateTypeInfoWithVTable(typeInfos[key], vtablePtr);
    }

    for (auto& instr : lir) {
        res.push_back(generate(instr.get()));
    }

    return res;
}