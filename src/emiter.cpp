#include "emiter.hpp"

std::unique_ptr<llvm::LLVMContext> emiterContext;
std::unique_ptr<llvm::Module> emiterModule;
std::unique_ptr<llvm::IRBuilder<>> emiterBuilder;

std::string mangleName(const std::string &name) {
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

llvm::Value* LLVMGenerator::generate(IRValue* node) {
    if (auto n = dynamic_cast<IRNumber*>(node)) {
        return llvm::ConstantFP::get(mapLLVMType(n->type), n->number);
    } else if (auto a = dynamic_cast<IRAlloca*>(node)) {
        llvm::Type* type = mapLLVMType(a->type);
        llvm::Value* allocaInstr = emiterBuilder.CreateAlloca(type, nullptr, a->name);
        namedValues[a] = allocaInstr;
        return allocaInstr;
    } else if (auto v = dynamic_cast<IRVariableRead*>(node)) {
        for (auto& [key, val] : namedValues) {
            if (key->name == v->name) {
                return emiterBuilder.CreateLoad(mapLLVMType(v->type), val, v->type);
            }
        }

        std::cerr << "Variable not found: " << v->name << "\n";
        return nullptr;
    } else if (auto s = dynamic_cast<IRStore*>(node)) {
        llvm::Value* ptr = generate(s->ptr);
        llvm::Value* val = generate(s->value);
        return emiterBuilder.CreateStore(val, ptr);
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

        return func;
    } else if (auto c = dynamic_cast<IRCall*>(node)) {
        llvm::Function* callee = emiterModule->getFunction(c->funcName);
        if (!callee) {
            std::cerr << "Function not found: " << c->funcName << "\n";
            return nullptr;
        }

        std::vector<llvm::Value*> args;
        for (auto argNode : c->args) {
            llvm::Value* argVal = generate(argNode);
            if (!argVal) return nullptr;
            args.push_back(argVal);
        }

        return emiterBuilder.CreateCall(callee, args, c->name);
    }
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
        }
    }

    for (auto& instr : lir) {
        res.push_back(generate(instr.get()));
    }

    return res;
}