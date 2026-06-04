#include "emiter/emiter.hpp"

std::unordered_map<std::string, Hash128> typeIds;
std::unordered_map<std::string, std::unordered_map<std::string, int>> structValues;
std::unordered_map<std::string, RTTIRecord> globalTypeInfos;

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

std::string getMethodName(const std::string& fn) {
    size_t pos = fn.rfind("_F");
    if (pos == std::string::npos) return fn;

    pos += 2;
    size_t lenEnd = pos;

    while (lenEnd < fn.size() && isdigit(fn[lenEnd])) {
        lenEnd++;
    }

    int len = std::stoi(fn.substr(pos, lenEnd - pos));
    return fn.substr(lenEnd, len);
}

bool hasReturn(llvm::Function* fn) {
    for (auto& block : *fn) {
        auto* term = block.getTerminator();

        if (llvm::isa<llvm::ReturnInst>(term)) {
            return true;
        }
    }

    return false;
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

    Hash128 id;
    if (typeIds.count(name)) id = typeIds[name];
    else id = hash128(name);

    globalTypeInfos[name] = {name, parentName, id};

    auto* gv = new llvm::GlobalVariable(*emiterModule, structTypes["_BI_TypeInfo"], true, llvm::GlobalValue::ExternalLinkage, nullptr, "_T" + name);

    typeInfos[name] = gv;
    return gv;
}

void LLVMGenerator::buildVTable(const std::string& cls) {
    if (vTables.count(cls)) return;

    std::vector<llvm::Function*> finalMethods;
    std::unordered_map<std::string, int> indexMap;

    for (const auto& parent : classHierarchy[cls]) {
        if (parent.empty()) continue;

        buildVTable(parent);

        const auto& parentV = vTables[parent];
        for (auto* fn : parentV) {
            std::string name = getMethodName(fn->getName().str());

            if (!indexMap.count(name)) {
                indexMap[name] = static_cast<int>(finalMethods.size());
                finalMethods.push_back(fn);
            }
        }
    }

    for (auto* fn : typeMethods[cls]) {
        std::string name = getMethodName(fn->getName().str());

        if (indexMap.count(name)) {
            finalMethods[indexMap[name]] = fn;
        } else {
            indexMap[name] = static_cast<int>(finalMethods.size());
            finalMethods.push_back(fn);
        }
    }

    for (size_t i = 0; i < finalMethods.size(); i++) {
        std::string name = getMethodName(finalMethods[i]->getName().str());
        vTablePos[cls][name] = static_cast<int>(i);
    }

    auto& info = globalTypeInfos[cls];

    info.vtableName = "_VT" + cls;
    info.vtableMethods.clear();

    for (auto* fn : finalMethods) {
        info.vtableMethods.push_back(fn->getName().str());
    }

    auto* vtableType = llvm::ArrayType::get(llvm::Type::getInt8PtrTy(emiterContext), finalMethods.size());
    auto* vTableGlobal = new llvm::GlobalVariable(*emiterModule, vtableType, true, llvm::GlobalValue::ExternalLinkage, nullptr, "_VT" + cls);

    vTables[cls] = finalMethods;
}

llvm::Value* LLVMGenerator::generate(IRValue* node) {
    if (auto n = dynamic_cast<IRNumber*>(node)) {
        auto num = llvm::ConstantFP::get(mapLLVMType(n->type), n->number);
        namedValues[n] = num;
        return num;
    } else if (auto a = dynamic_cast<IRAlloca*>(node)) {
        llvm::Type* type = mapLLVMType(a->type);

        llvm::Value* ptr = emiterBuilder.CreateAlloca(type);

        namedValues[a] = ptr;
        return ptr;
    } else if (auto v = dynamic_cast<IRVariableRead*>(node)) {
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
        auto* gep = emiterBuilder.CreateStructGEP(mapLLVMType(a->object->type, false), obj, structValues[a->object->type]["$" + std::to_string(a->memberName)]);
        namedValues[a] = gep;
        return gep;
    } else if (auto g = dynamic_cast<IRAllocaStruct*>(node)) {
        llvm::Type* type = mapLLVMType(g->type, false);

        llvm::Function* callee = emiterModule->getFunction("_BI_malloc");        
        if (!callee) return nullptr;

        const llvm::DataLayout &dl = emiterModule->getDataLayout();
        uint64_t totalSize = dl.getTypeAllocSize(type);

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

            llvm::StructType* namedStruct =generateStruct(c->name, body, parents);
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

std::unique_ptr<llvm::Module> generateRTTIModule(llvm::LLVMContext& ctx, const std::unordered_map<std::string, RTTIRecord>& globalTypeInfos, const std::string& moduleName) {
    auto module = std::make_unique<llvm::Module>(moduleName, ctx);
    auto* typeInfoTy = llvm::StructType::create(ctx, {llvm::Type::getInt128Ty(ctx), llvm::PointerType::getUnqual(ctx), llvm::Type::getInt8PtrTy(ctx)->getPointerTo()}, "_BI_TypeInfo");

    std::unordered_map<std::string, llvm::GlobalVariable*> typeInfos;
    std::unordered_set<std::string> declaredFunctions;

    for (auto& [_, info] : globalTypeInfos) {
        for (auto& fnName : info.vtableMethods) {
            if (declaredFunctions.contains(fnName)) continue;
            declaredFunctions.insert(fnName);

            auto* fnTy = llvm::FunctionType::get(llvm::Type::getInt8PtrTy(ctx), true);
            llvm::Function::Create(
                fnTy,
                llvm::GlobalValue::ExternalLinkage,
                fnName,
                module.get()
            );
        }
    }

    for (auto& [name, _] : globalTypeInfos) {
        auto* gv = new llvm::GlobalVariable(*module, typeInfoTy, true, llvm::GlobalValue::ExternalLinkage, nullptr, "_T" + name);
        typeInfos[name] = gv;
    }

    std::unordered_map<std::string, llvm::GlobalVariable*> vtables;
    for (auto& [name, info] : globalTypeInfos) {
        std::vector<llvm::Constant*> entries;

        for (auto& fnName : info.vtableMethods) {
            auto* fn = module->getFunction(fnName);
            entries.push_back(llvm::ConstantExpr::getBitCast(fn, llvm::Type::getInt8PtrTy(ctx)));
        }

        auto* arrTy = llvm::ArrayType::get(llvm::Type::getInt8PtrTy(ctx), entries.size());
        auto* arr = llvm::ConstantArray::get(arrTy, entries);

        auto* gv = new llvm::GlobalVariable(*module, arrTy, true, llvm::GlobalValue::ExternalLinkage, arr, info.vtableName);
        vtables[name] = gv;
    }

    for (auto& [name, info] : globalTypeInfos) {

        llvm::Constant* parent = llvm::ConstantPointerNull::get(typeInfoTy->getPointerTo());

        if (!info.parent.empty()) {
            parent = typeInfos[info.parent];
        }

        llvm::Constant* idConst = llvm::ConstantInt::get(
            llvm::Type::getInt128Ty(ctx),
            toAPInt(info.id)
        );

        llvm::Constant* vtablePtr = llvm::ConstantExpr::getBitCast(
            vtables[name],
            llvm::Type::getInt8PtrTy(ctx)->getPointerTo()
        );

        auto* init = llvm::ConstantStruct::get(typeInfoTy,
            {
                idConst,
                parent,
                vtablePtr
            }
        );

        typeInfos[name]->setInitializer(init);
    }

    return module;
}
