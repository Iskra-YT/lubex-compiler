#include "LIR/lir.hpp"
#include "emiter/emiter.hpp"
#include <filesystem>

std::string mangleVisitor = "";
extern std::filesystem::path mainSource;

int lastId = 0;
int lastClassId = 0;
static std::unordered_map<std::string, IRValue*> variables;

static Context* currentContext = nullptr;

std::string getType(IdentyfierNode* name, Symbol* sym) {
    if (name->value == "Number") {
        return "_BI_Number";
    } else if (name->value == "Void") {
        return "_BI_Void";
    } else if (name->value == "Object") {
        return "_BI_Object";
    } else if (name->value == "Null") {
        return "void*";
    }

    return mangleName(sym);
}

LIRGenerate parse(ASTNode* node);
Symbol* resolveCallChain(ASTNode* node, IRValue*& baseIR, bool callBase = false, bool isCall = false);

LIRGenerate parseLValue(ASTNode* node) {
    if (auto id = dynamic_cast<IdentyfierNode*>(node)) {
        auto sym = currentContext->lookup(id);
        auto it = variables.find(mangleName(sym));

        if (it == variables.end()) {
            throw LIRException(Error(id->position, "Undefined variable", mainSource.filename().string()));
        }

        return { it->second, {} };
    }

    if (auto access = dynamic_cast<MemberAccessNode*>(node)) {
        IRValue* baseIR = nullptr;
        Symbol* memberSym = resolveCallChain(access, baseIR);

        if (!baseIR) {
            throw LIRException(Error(access->position, "No base object", mainSource.filename().string()));
        }

        std::vector<std::unique_ptr<IRValue>> res;

        auto accessIr = std::make_unique<IRAccess>(
            "%" + std::to_string(lastId++),
            getType(memberSym->type->name, memberSym),
            baseIR,
            std::to_string(memberSym->classMemberIndex)
        );

        IRValue* ptr = accessIr.get();
        res.push_back(std::move(accessIr));

        return { ptr, std::move(res) };
    }

    throw LIRException(Error(node->position, "Invalid LValue", mainSource.filename().string()));
}

inline IRArg* createArg(const std::string& type) {
    return new IRArg("%" + std::to_string(lastId++), type);
}

IRValue* parseArg(ASTNode* node) {
    auto arg = static_cast<ArgDeclaration*>(node);
    auto argSym = currentContext->lookup(static_cast<IdentyfierNode*>(arg->name.get()));
    auto argIr = createArg(getType(argSym->type->name, argSym->type));
    variables[mangleName(argSym)] = argIr;
    return argIr;
}

LIRGenerate parseNumbers(NumberNode* num) {
    auto n = std::make_unique<IRNumber>("%" + std::to_string(lastId++), "double", num->value);
    auto alloc = std::make_unique<IRAllocaStruct>("%" + std::to_string(lastId++), "_BI_Number");

    std::vector<IRValue*> args;
    args.push_back(alloc.get());
    args.push_back(n.get());
    auto call = std::make_unique<IRCall>("_BI_Number_init", "_BI_Number", args);

    std::vector<std::unique_ptr<IRValue>> code;
    code.push_back(std::move(alloc));
    code.push_back(std::move(n));
    code.push_back(std::move(call));
    return {code.back().get(), std::move(code)};
}

LIRGenerate parseVariableDeclaration(VariableDeclarationNode* decl) {
    auto varSym = currentContext->lookup(static_cast<IdentyfierNode*>(decl->name.get()));
    
    std::string typeName = getType(varSym->type->name, varSym->type);
    auto alloca = std::make_unique<IRAlloca>("%" + std::to_string(lastId++), typeName);
    IRValue* allocaPtr = alloca.get();
    variables[mangleName(varSym)] = allocaPtr;

    if (decl->value.get()) {
        std::vector<std::unique_ptr<IRValue>> res;

        auto allocaPtr = alloca.get();
        res.push_back(std::move(alloca));

        auto valIR = parse(decl->value.get());
        auto store = std::make_unique<IRStore>(allocaPtr, valIR.mainValue);

        for (auto& insr : valIR.code) {
            res.push_back(std::move(insr));
        }

        res.push_back(std::move(store));

        return {
            allocaPtr,
            std::move(res)
        };
    }

    std::vector<std::unique_ptr<IRValue>> body;
    body.push_back(std::move(alloca));
    return {
        alloca.get(),
        std::move(body)
    };
}

LIRGenerate parseVariableAssigment(VariableAssigment* assign) {
    std::vector<std::unique_ptr<IRValue>> res;
    IRValue* base = nullptr;
    Symbol* sym = resolveCallChain(assign->name.get(), base);
    
    auto valIR = parse(assign->value.get());

    for (auto& instr : valIR.code) {
        res.push_back(std::move(instr));
    }

    auto lhs = parseLValue(assign->name.get());

    for (auto& instr : lhs.code) {
        res.push_back(std::move(instr));
    }
    
    IRValue* allocaPtr = lhs.mainValue;

    auto store = std::make_unique<IRStore>(allocaPtr, valIR.mainValue);

    res.push_back(std::move(store));
    return {
        res.back().get(),
        std::move(res)
    };
}

LIRGenerate parseBinaryExpression(BinaryNode* bin) {
    Symbol* binSym = bin->evaluateSymbol(*currentContext);
    std::vector<std::unique_ptr<IRValue>> res;
    auto L = parse(bin->left.get());
    auto R = parse(bin->right.get());

    for (auto& instr : L.code) {
        res.push_back(std::move(instr));
    }

    for (auto& instr : R.code) {
        res.push_back(std::move(instr));
    }

    // TODO: Add Object type support
    if (bin->op == "+") {
        if (binSym->type->name->value == "Number") {
            res.push_back(std::make_unique<IRCall>("_BI_Number_add", "_BI_Number", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else if (binSym->type->name->value == "Void") {
            res.push_back(std::make_unique<IRCall>("_BI_Void_add", "_BI_Void", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else {
            res.push_back(std::make_unique<IRCall>(mangleName(binSym->type) + "_F3add", mangleName(binSym->type), std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        }
    } else if (bin->op == "-") {
        if (binSym->type->name->value == "Number") {
            res.push_back(std::make_unique<IRCall>("_BI_Number_subtract", "_BI_Number", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else if (binSym->type->name->value == "Void") {
            res.push_back(std::make_unique<IRCall>("_BI_Void_subtract", "_BI_Void", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else {
            res.push_back(std::make_unique<IRCall>(mangleName(binSym->type) + "_F8subtract", mangleName(binSym->type), std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        }
    } else if (bin->op == "*") {
        if (binSym->type->name->value == "Number") {
            res.push_back(std::make_unique<IRCall>("_BI_Number_multiply", "_BI_Number", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else if (binSym->type->name->value == "Void") {
            res.push_back(std::make_unique<IRCall>("_BI_Void_multiply", "_BI_Void", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else {
            res.push_back(std::make_unique<IRCall>(mangleName(binSym->type) + "_F8multiply", mangleName(binSym->type), std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        }
    } else if (bin->op == "/") {
        if (binSym->type->name->value == "Number") {
            res.push_back(std::make_unique<IRCall>("_BI_Number_divide", "_BI_Number", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else if (binSym->type->name->value == "Void") {
            res.push_back(std::make_unique<IRCall>("_BI_Void_divide", "_BI_Void", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else {
            res.push_back(std::make_unique<IRCall>(mangleName(binSym->type) + "_F6divide", mangleName(binSym->type), std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        }
    } else if (bin->op == "==") {
        if (binSym->type->name->value == "Number") {
            res.push_back(std::make_unique<IRCall>("_BI_Number_equals", "_BI_Number", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else {
            res.push_back(std::make_unique<IRCall>("_BI_Number_equals", "_BI_Number", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        }
    } else if (bin->op == "!=") {
        if (binSym->type->name->value == "Number") {
            res.push_back(std::make_unique<IRCall>("_BI_Number_notEquals", "_BI_Number", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else {
            res.push_back(std::make_unique<IRCall>("_BI_Number_notEquals", "_BI_Number", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        }
    } else if (bin->op == "<") {
        if (binSym->type->name->value == "Number") {
            res.push_back(std::make_unique<IRCall>("_BI_Number_lessThan", "_BI_Number", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else {
            res.push_back(std::make_unique<IRCall>("_BI_Number_lessThan", "_BI_Number", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        }
    } else if (bin->op == ">") {
        if (binSym->type->name->value == "Number") {
            res.push_back(std::make_unique<IRCall>("_BI_Number_greaterThan", "_BI_Number", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else {
            res.push_back(std::make_unique<IRCall>("_BI_Number_greaterThan", "_BI_Number", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        }
    } else if (bin->op == "<=") {
        if (binSym->type->name->value == "Number") {
            res.push_back(std::make_unique<IRCall>("_BI_Number_lessOrEqual", "_BI_Number", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else {
            res.push_back(std::make_unique<IRCall>("_BI_Number_lessOrEqual", "_BI_Number", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        }
    } else if (bin->op == ">=") {
        if (binSym->type->name->value == "Number") {
            res.push_back(std::make_unique<IRCall>("_BI_Number_greaterOrEqual", "_BI_Number", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else {
            res.push_back(std::make_unique<IRCall>("_BI_Number_greaterOrEqual", "_BI_Number", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        }
    }

    return {};
}

LIRGenerate parseUnaryExpression(UnaryNode* un) {
    std::vector<std::unique_ptr<IRValue>> res;
    auto V = parse(un->value.get());

    for (auto& instr : V.code) {
        res.push_back(std::move(instr));
    }

    if (un->op == "!") {
        res.push_back(std::make_unique<IRCall>("_BI_Number_logicalNot", "_BI_Number", std::vector<IRValue*>{V.mainValue}));
        return {res.back().get(), std::move(res)};
    } else if (un->op == "~") {
        res.push_back(std::make_unique<IRCall>("_BI_Number_bitwiseNot", "_BI_Number", std::vector<IRValue*>{V.mainValue}));
        return {res.back().get(), std::move(res)};
    }

    return {};
}

LIRGenerate parseFunction(FunctionDeclaration* func) {
    auto funcSym = currentContext->lookup(static_cast<IdentyfierNode*>(func->name.get())); 
    std::vector<std::unique_ptr<IRValue>> res;
    std::vector<IRValue*> args;

    lastId = 0;

    if (!func->isStatic) {
        variables[mangleName(funcSym->mangledName + ".this")] = createArg(mangleName(funcSym->scope->parent->generativeSymbol));
        args.push_back(variables[mangleName(funcSym->mangledName + ".this")]);
    }

    for (auto& arg : func->parameters) {
        currentContext = funcSym->scope;
        auto argIr = parseArg(arg.get());
        args.push_back(argIr);
    }

    std::string typeName = getType(funcSym->type->name, funcSym->type);
    auto funcIr = std::make_unique<IRFunction>(mangleName(funcSym), std::move(args), typeName, mangleName(funcSym->scope->parent->generativeSymbol), funcSym->isStatic);
    for (auto& node : func->body) {
        currentContext = funcSym->scope;
        auto ir = parse(node.get());
        for (auto& instr : ir.code) {
            funcIr->body.push_back(std::move(instr));
        }
    }

    auto funcIrPtr = funcIr.get();
    res.push_back(std::move(funcIr));
    return {
        funcIrPtr,
        std::move(res)
    };
}

LIRGenerate parseClassDeclaration(ClassDeclNode* cls) {
    auto classSym = currentContext->lookup(static_cast<IdentyfierNode*>(cls->name.get()));
    std::vector<std::unique_ptr<IRValue>> body;
    std::vector<std::unique_ptr<IRValue>> structBody;

    lastClassId = 0;

    for (auto& node : cls->members) {
        if (auto var = dynamic_cast<VariableDeclarationNode*>(dynamic_cast<StatementNode*>(node.get())->value.get())) {
            auto varSym = classSym->scope->lookup(dynamic_cast<IdentyfierNode*>(var->name.get()));

            std::string typeName = getType(varSym->type->name, varSym->type);

            auto member = std::make_unique<IRMember>("$" + std::to_string(lastClassId++), typeName, lastClassId - 1);
            varSym->classMemberIndex = lastClassId - 1;
            structBody.push_back(std::move(member));

            variables[mangleName(varSym)] = structBody.back().get();
            continue;
        }

        currentContext = classSym->scope;
        auto code = parse(node.get()).code;
        for (auto& instr : code) {
            body.push_back(std::move(instr));
        }
    }

    auto stc = std::make_unique<IRStruct>(mangleName(classSym), std::move(structBody), classSym->classTypes.size() != 0 ? mangleName(classSym->classTypes[0]) : "");
    body.insert(body.begin(), std::move(stc));
    variables[mangleName(classSym)] = body[0].get();

    return {
        body[0].get(),
        std::move(body)
    };
}

LIRGenerate parseIdentyfierNode(IdentyfierNode* id) {
    auto idSym = currentContext->lookup(id);
    if (idSym->kind == SymbolKind::CLASS) {
        return { new IRClass(mangleName(idSym), idSym->name->value), {} };
    }

    auto it = variables.find(mangleName(idSym));
    if (it == variables.end()) {
        throw LIRException(Error(id->position, "Undefined variable: " + id->value + " in LIR", mainSource.filename().string()));
    }

    auto allocSym = it->second;

    auto var = std::make_unique<IRVariableRead>(allocSym->name, allocSym->type);

    std::vector<std::unique_ptr<IRValue>> res;
    res.push_back(std::move(var));

    return { res.back().get(), std::move(res) };
}

extern IdentyfierNode* initName;
extern IdentyfierNode objectType;
extern Symbol* lookupWithInheritance(Symbol* cls, const std::string& name, Symbol* objectClass, bool isCall = false);

Symbol* resolveCallChain(ASTNode* node, IRValue*& baseIR, bool callBase, bool isCall) {
    if (auto id = dynamic_cast<IdentyfierNode*>(node)) {
        Symbol* sym = currentContext->lookup(id);
        if (!sym) {
            throw LIRException(Error(id->position, "Unknown identifier: " + id->value, mainSource.filename().string()));
        }

        auto it = variables.find(mangleName(sym));
        baseIR = (it != variables.end()) ? it->second : nullptr;

        if (callBase && sym->kind == SymbolKind::CLASS) {
            Symbol* initSym = lookupWithInheritance(sym, "init", currentContext->lookup(&objectType));
            
            if (!initSym) {
                throw LIRException(Error(id->position, "Class " + id->value + " has no init method", mainSource.filename().string()));
            }
        
            baseIR = nullptr;
            initSym->type = sym;
            return initSym;
        }

        return sym;
    } else if (auto access = dynamic_cast<MemberAccessNode*>(node)) {
        IRValue* leftIR = nullptr;
        Symbol* leftSym = resolveCallChain(access->object.get(), leftIR, false, isCall);

        if (!leftSym) {
            throw LIRException(Error(node->position, "Cannot resolve call chain", mainSource.filename().string()));
        }

        auto memberId = static_cast<IdentyfierNode*>(access->member.get());
        const std::string& memberName = memberId->value;

        Symbol* classToSearch = nullptr;

        if (leftSym->kind == SymbolKind::CLASS || leftSym->kind == SymbolKind::MODULE) {
            classToSearch = leftSym;
        } else {
            if (!leftSym->type) {
                throw LIRException(Error(node->position, "Invalid object in call chain", mainSource.filename().string()));
            }
            classToSearch = leftSym->type;
        }

        if (!classToSearch || !classToSearch->scope) {
            throw LIRException(Error(node->position, "Class/module has no scope", mainSource.filename().string()));
        }

        Symbol* memberSym = lookupWithInheritance(classToSearch, memberName, currentContext->lookup(&objectType));

        if (!memberSym) {
            throw LIRException(Error(node->position, "Unknown member in call chain: " + memberName, mainSource.filename().string()));
        }

        baseIR = leftIR;
        return memberSym;
    } else if (auto th = dynamic_cast<ThisNode*>(node)) {
        if (currentContext->generativeSymbol->isStatic) {
            throw LIRException(Error(node->position, "'this' in static context", mainSource.filename().string()));
        }

        auto classSym = currentContext->parent->generativeSymbol;

        baseIR = variables[
            mangleName(currentContext->generativeSymbol->mangledName + ".this")
        ];

        return classSym;
    }

    throw LIRException(Error(node->position, "Invalid node in call chain", mainSource.filename().string()));
}

LIRGenerate parseCallNode(CallNode* call) {
    IRValue* baseObject = nullptr;
    Symbol* callSym = resolveCallChain(call->callee.get(), baseObject, true, true);

    std::vector<std::unique_ptr<IRValue>> res;
    std::vector<IRValue*> args;


    if (!callSym->isStatic) {
        if (!baseObject) {
            if (currentContext->generativeSymbol->isStatic) {
                throw LIRException(Error(call->position, "Cannot call non-static method from static context", mainSource.filename().string()));
            }
            baseObject = new IRVariableRead("%0", mangleName(currentContext->parent->generativeSymbol));
        } else {
            if (dynamic_cast<IRAlloca*>(baseObject) || dynamic_cast<IRAccess*>(baseObject)) {
                auto load = std::make_unique<IRVariableRead>(baseObject->name, baseObject->type);
                baseObject = load.get();
                res.push_back(std::move(load));
            }
        }
        
        if (callSym->name->value != "init")
            args.push_back(baseObject);
    }

    for (auto& arg : call->args) {
        auto argIR = parse(arg.get());
        for (auto& instr : argIR.code) res.push_back(std::move(instr));
        args.push_back(argIR.mainValue);
    }

    if (callSym->name->value == "init" && callSym->kind == SymbolKind::FUNCTION) {
        auto ownerClass = callSym->type;
        auto alloc = std::make_unique<IRAllocaStruct>("%" + std::to_string(lastId++), mangleName(ownerClass));

        IRValue* objPtr = alloc.get();
        res.push_back(std::move(alloc));
        args.insert(args.begin(), objPtr);

        auto callIR = std::make_unique<IRCall>(mangleName(callSym), mangleName(ownerClass), args);
        res.push_back(std::move(callIR));
    } else {
        auto callIR = std::make_unique<IRCall>(mangleName(callSym), getType(callSym->type->name, callSym->type), args);
        res.push_back(std::move(callIR));
    }

    return { res.back().get(), std::move(res) };
}

LIRGenerate parseMemberAccessNode(MemberAccessNode* access) {
    auto lval = parseLValue(access);

    std::vector<std::unique_ptr<IRValue>> res;
    for (auto& instr : lval.code) {
        res.push_back(std::move(instr));
    }

    auto ptr = lval.mainValue;

    auto load = std::make_unique<IRVariableRead>(
        ptr->name,
        ptr->type
    );
    IRValue* val = load.get();

    res.push_back(std::move(load));
    return { val, std::move(res) };
}

LIRGenerate parseReturnNode(ReturnNode* ret) {
    std::vector<std::unique_ptr<IRValue>> res;
    LIRGenerate valueIr;
    if (ret->value) valueIr = parse(ret->value.get());
    else {
        auto alloc = std::make_unique<IRAllocaStruct>("%" + std::to_string(lastId++), "_BI_Void");
        res.push_back(std::move(alloc));

        auto ir = std::make_unique<IRCall>("_BI_Void_init", "_BI_Void", std::vector<IRValue*>{res.back().get()});
        auto resIr = std::vector<std::unique_ptr<IRValue>>{};
        resIr.push_back(std::move(ir));
        
        valueIr = { resIr.back().get(), std::move(resIr) };
    }

    for (auto& instr : valueIr.code) {
        res.push_back(std::move(instr));
    }

    auto returnIR = std::make_unique<IRReturn>(valueIr.mainValue->type, valueIr.mainValue);
    res.push_back(std::move(returnIR));

    return {
        res.back().get(),
        std::move(res)
    };
}

LIRGenerate parseAttributes(AttributesNode* attr) {
    if (static_cast<IdentyfierNode*>(attr->name.get())->value == "mangle") {
        if (auto fn = dynamic_cast<FunctionDeclaration*>(attr->value.get())) {
            auto sym = currentContext->lookup(static_cast<IdentyfierNode*>(fn->name.get()));
            sym->forcedMangle = static_cast<IdentyfierNode*>(attr->params[0].get())->value;
        }

        return parse(attr->value.get());
    } else {
        throw LIRException(Error(attr->position, "Unknown attribute: " + static_cast<IdentyfierNode*>(attr->name.get())->value, mainSource.filename().string()));
    }
}

LIRGenerate parseImport(ImportNode* imp) {
    lastId = 0;
    auto name = static_cast<IdentyfierNode*>(imp->value.get());
    auto impSym = currentContext->lookup(name);
    if (!impSym || !impSym->scope) {
        throw LIRException(Error(imp->position, "Cannot import unknown module: " + name->value, mainSource.filename().string()));
    }

    std::vector<std::unique_ptr<IRValue>> res;

    for (auto& [symName, sym] : impSym->scope->symbols) {
        if (sym->kind == SymbolKind::CLASS) {
            auto structIR = std::make_unique<IRStruct>(mangleName(sym.get()), std::vector<std::unique_ptr<IRValue>>{}, sym->classTypes.size() != 0 ? mangleName(sym->classTypes[0]) : "");
            variables[mangleName(sym.get())] = structIR.get();
            res.push_back(std::move(structIR));

            auto savedCtx = currentContext;
            currentContext = sym->scope;

            for (auto& [methodName, methodSym] : sym->scope->symbols) {
                if (methodSym->kind == SymbolKind::FUNCTION) {
                    std::vector<IRValue*> args;
                    auto funcNode = static_cast<FunctionDeclaration*>(methodSym->node);

                    for (auto& param : funcNode->parameters) {
                        auto savedCtxInner = currentContext;
                        currentContext = methodSym->scope;
                        auto argIR = parseArg(param.get());
                        args.push_back(argIR);
                        currentContext = savedCtxInner;
                    }

                    std::string typeName;
                    if (methodSym->type->name->value == "Number") typeName = "_BI_Number";
                    else if (methodSym->type->name->value == "Void") typeName = "_BI_Void";
                    else typeName = mangleName(methodSym->type);

                    auto funcIR = std::make_unique<IRFunction>(mangleName(methodSym.get()), args, typeName, mangleName(methodSym->scope->parent->generativeSymbol), methodSym->isStatic);
                    variables[mangleName(methodSym.get())] = funcIR.get();
                    res.push_back(std::move(funcIR));
                }
            }

            currentContext = savedCtx;
        }
    }

    return { nullptr, std::move(res) };
}

LIRGenerate parseString(StringNode* str) {
    auto strIR = std::make_unique<IRString>("%" + std::to_string(lastId++), "carr", str->value);
    std::vector<std::unique_ptr<IRValue>> res;

    auto alloc = std::make_unique<IRAllocaStruct>("%" + std::to_string(lastId++), "_BI_String");
    res.push_back(std::move(alloc));
    
    auto call = std::make_unique<IRCall>("_BI_String_init", "_BI_String", std::vector<IRValue*>{res.back().get(), strIR.get()});
    res.push_back(std::move(strIR));
    res.push_back(std::move(call));
    return { res.back().get(), std::move(res) };
}

LIRGenerate parseNullNode(NullNode* node) {
    auto nullIR = std::make_unique<IRNull>("%" + std::to_string(lastId++), "void*");
    IRValue* ptr = nullIR.get();
    std::vector<std::unique_ptr<IRValue>> res;
    res.push_back(std::move(nullIR));
    return { ptr, std::move(res) };
}

LIRGenerate parseNullCoalescing(NullCoalescingNode* coalesce) {
    std::vector<std::unique_ptr<IRValue>> res;
    auto L = parse(coalesce->left.get());
    auto R = parse(coalesce->right.get());

    for (auto& instr : L.code) res.push_back(std::move(instr));
    for (auto& instr : R.code) res.push_back(std::move(instr));

    auto result = std::make_unique<IRNullCoalescing>(
        L.mainValue->type,
        L.mainValue,
        R.mainValue
    );
    IRValue* ptr = result.get();
    res.push_back(std::move(result));
    return { ptr, std::move(res) };
}

LIRGenerate parseNullCheck(NullCheckNode* check) {
    std::vector<std::unique_ptr<IRValue>> res;
    auto valIR = parse(check->value.get());
    for (auto& instr : valIR.code) res.push_back(std::move(instr));

    auto result = std::make_unique<IRNullCheck>(
        valIR.mainValue->type,
        valIR.mainValue
    );
    IRValue* ptr = result.get();
    res.push_back(std::move(result));
    return { ptr, std::move(res) };
}

LIRGenerate parseSafeNavigationNode(SafeNavigationNode* safe) {
    std::vector<std::unique_ptr<IRValue>> res;

    auto objIR = parse(safe->object.get());
    for (auto& instr : objIR.code) res.push_back(std::move(instr));

    auto objSym = safe->object->evaluateSymbol(*currentContext);
    if (!objSym || !objSym->type) {
        throw LIRException(Error(safe->position, "Cannot resolve safe access", mainSource.filename().string()));
    }

    Symbol* objType = objSym->type;
    if (objType->name->value == "Null") {
        throw LIRException(Error(safe->position, "Cannot access member on null", mainSource.filename().string()));
    }

    std::string memberName = static_cast<IdentyfierNode*>(safe->member.get())->value;
    Symbol* objectClass = currentContext->lookup(&objectType);
    Symbol* memberSym = lookupWithInheritance(objType, memberName, objectClass);
    if (!memberSym) {
        throw LIRException(Error(safe->position, "No such member: " + memberName, mainSource.filename().string()));
    }

    if (memberSym->kind == SymbolKind::FUNCTION) {
        throw LIRException(Error(safe->position, "Safe navigation on methods is not yet supported", mainSource.filename().string()));
    }

    std::string memberTypeName = memberSym->type ? memberSym->type->name->value : "Void";
    std::string memberType = getType(memberSym->type->name, memberSym->type);

    auto safeIr = std::make_unique<IRSafeAccess>(
        "%" + std::to_string(lastId++),
        memberType,
        objIR.mainValue,
        memberSym->classMemberIndex
    );
    IRValue* result = safeIr.get();
    res.push_back(std::move(safeIr));

    return { result, std::move(res) };
}

LIRGenerate parseThisNode(ThisNode* node) {
    if (!currentContext || !currentContext->generativeSymbol) {
        throw LIRException(Error(node->position, "'this' used outside of context", mainSource.filename().string()));
    }

    if (currentContext->generativeSymbol->isStatic) {
        throw LIRException(Error(node->position, "'this' cannot be used in static context", mainSource.filename().string()));
    }

    auto classSym = currentContext->parent->generativeSymbol;
    auto thisVal = std::make_unique<IRVariableRead>("%0", mangleName(classSym));

    std::vector<std::unique_ptr<IRValue>> res;
    res.push_back(std::move(thisVal));

    return { res.back().get(), std::move(res) };
}

LIRGenerate parse(ASTNode* node) {
    if (auto stmt = dynamic_cast<StatementNode*>(node)) {
        return parse(stmt->value.get());
    }

    if (auto num = dynamic_cast<NumberNode*>(node)) {
        return parseNumbers(num);
    } else if (auto decl = dynamic_cast<VariableDeclarationNode*>(node)) {
        return parseVariableDeclaration(decl);
    } else if (auto assign = dynamic_cast<VariableAssigment*>(node)) {
        return parseVariableAssigment(assign);
    } else if (auto bin = dynamic_cast<BinaryNode*>(node)) {
        return parseBinaryExpression(bin);
    } else if (auto un = dynamic_cast<UnaryNode*>(node)) {
        return parseUnaryExpression(un);
    } else if (auto func = dynamic_cast<FunctionDeclaration*>(node)) {
        return parseFunction(func);
    } else if (auto cls = dynamic_cast<ClassDeclNode*>(node)) {
        return parseClassDeclaration(cls);
    } else if (auto id = dynamic_cast<IdentyfierNode*>(node)) {
        return parseIdentyfierNode(id);
    } else if (auto call = dynamic_cast<CallNode*>(node)) {
        return parseCallNode(call);
    } else if (auto access = dynamic_cast<MemberAccessNode*>(node)) {
        return parseMemberAccessNode(access);
    } else if (auto ret = dynamic_cast<ReturnNode*>(node)) {
        return parseReturnNode(ret);
    } else if (auto nullNode = dynamic_cast<NullNode*>(node)) {
        return parseNullNode(nullNode);
    } else if (auto coalesce = dynamic_cast<NullCoalescingNode*>(node)) {
        return parseNullCoalescing(coalesce);
    } else if (auto check = dynamic_cast<NullCheckNode*>(node)) {
        return parseNullCheck(check);
    } else if (auto safe = dynamic_cast<SafeNavigationNode*>(node)) {
        return parseSafeNavigationNode(safe);
    } else if (auto attr = dynamic_cast<AttributesNode*>(node)) {
        return parseAttributes(attr);
    } else if (auto mod = dynamic_cast<ModuleDeclaration*>(node)) {
        currentContext = currentContext->lookup(static_cast<IdentyfierNode*>(mod->name.get()), false)->scope;
    } else if (auto imp = dynamic_cast<ImportNode*>(node)) {
        return parseImport(imp);
    } else if (auto str = dynamic_cast<StringNode*>(node)) {
        return parseString(str);
    } else if (auto th = dynamic_cast<ThisNode*>(node)) {
        return parseThisNode(th);
    }

    return {};
}

std::vector<std::unique_ptr<IRValue>> generateLIR(std::vector<std::unique_ptr<ASTNode>>& nodes, Context* ctx) {
    lastId = 0;
    std::vector<std::unique_ptr<IRValue>> ir;
    currentContext = ctx;
    for (auto& node : nodes) {
        auto chunk = parse(node.get());

        for (auto& instr : chunk.code) {
            if (instr) {
                ir.push_back(std::move(instr));
            }
        }
    }

    return ir;
}