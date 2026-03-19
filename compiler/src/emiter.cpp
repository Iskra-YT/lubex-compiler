#include "emiter.hpp"

std::unique_ptr<llvm::LLVMContext> emiterContext;
std::unique_ptr<llvm::Module> emiterModule;
std::unique_ptr<llvm::IRBuilder<>> emiterBuilder;

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

        if (f->returnType == "_BI_Void") {
            emiterBuilder.CreateRetVoid();
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
            if (!argVal) return nullptr;
            args.push_back(argVal);
        }

        auto call = emiterBuilder.CreateCall(callee, args);
        namedValues[c] = call;

        return call;
    } else if (auto r = dynamic_cast<IRReturn*>(node)) {
        llvm::Value* ret;
        if (r->type == "_BI_Void") {
            ret = emiterBuilder.CreateRetVoid();
        } else {
            llvm::Value* value = namedValues[r->value];
            ret = emiterBuilder.CreateRet(value);
        }

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
        } else if (auto c = dynamic_cast<IRStruct*>(instr.get())) {
            std::vector<llvm::Type*> body;
            for (auto& type : c->data) {
                auto member = dynamic_cast<IRMember*>(type.get());
                if (!member) continue;

                body.push_back(mapLLVMType(member->type));
            }

            llvm::StructType* namedStruct = llvm::StructType::create(emiterContext, c->name);
            namedStruct->setBody(body);
            structTypes[c->name] = namedStruct;
        }
    }

    for (auto& instr : lir) {
        res.push_back(generate(instr.get()));
    }

    return res;
}