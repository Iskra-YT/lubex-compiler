#include "LIR/lir.hpp"
#include "emiter.hpp"

int lastId = 0;
int lastClassId = 0;
static std::unordered_map<std::string, IRValue*> variables;

LIRGenerate parse(std::unique_ptr<ASTNode> node, Context& ctx);

inline IRArg* createArg(const std::string& type) {
    return new IRArg("%" + std::to_string(lastId++), type);
}

IRValue* parseArg(std::unique_ptr<ASTNode> node, Context& ctx) {
    auto arg = static_cast<ArgDeclaration*>(node.get());
    auto argSym = ctx.lookup(static_cast<IdentyfierNode*>(arg->name.get()));
    auto argIr = createArg(static_cast<IdentyfierNode*>(arg->type.get())->value);
    variables[mangleName(argSym->mangledName)] = argIr;
    return argIr;
}

LIRGenerate parseNumbers(NumberNode* num, Context& ctx) {
    auto n = std::make_unique<IRNumber>("%" + std::to_string(lastId++), "double", num->value);

    std::vector<IRValue*> args;
    args.push_back(n.get());
    auto call = std::make_unique<IRCall>("_BI_Int_init", "_BI_Int", args);

    std::vector<std::unique_ptr<IRValue>> code;
    code.push_back(std::move(n));
    code.push_back(std::move(call));
    return {code.back().get(), std::move(code)};
}

LIRGenerate parseVariableDeclaration(VariableDeclarationNode* decl, Context& ctx) {
    auto varSym = ctx.lookup(static_cast<IdentyfierNode*>(decl->name.get()));
    
    std::string typeName;
    if (varSym->type->name->value == "Int") {
        typeName = "_BI_Int";
    } else if (varSym->type->name->value == "Void") {
        typeName = "_BI_Void";
    } else {
        typeName = mangleName(varSym->type->mangledName);
    }
    auto alloca = std::make_unique<IRAlloca>("%" + std::to_string(lastId++), typeName);
    IRValue* allocaPtr = alloca.get();
    variables[mangleName(varSym->mangledName)] = allocaPtr;

    if (decl->value.get()) {
        std::vector<std::unique_ptr<IRValue>> res;
        auto valIR = parse(std::move(decl->value), ctx);
        auto store = std::make_unique<IRStore>(allocaPtr, valIR.mainValue);

        for (auto& insr : valIR.code) {
            res.push_back(std::move(insr));
        }
        res.push_back(std::move(alloca));
        res.push_back(std::move(store));

        return {
            alloca.get(),
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

LIRGenerate parseVariableAssigment(VariableAssigment* assign, Context& ctx) {
    std::vector<std::unique_ptr<IRValue>> res;
    auto assignSym = ctx.lookup(static_cast<IdentyfierNode*>(assign->name.get()));
    auto valIR = parse(std::move(assign->value), ctx);

    for (auto& instr : valIR.code) {
        res.push_back(std::move(instr));
    }

    auto allocaPtr = variables[mangleName(assignSym->mangledName)];
    auto store = std::make_unique<IRStore>(allocaPtr, valIR.mainValue);

    res.push_back(std::move(store));
    return {
        res.back().get(),
        std::move(res)
    };
}

LIRGenerate parseBinaryExpression(BinaryNode* bin, Context& ctx) {
    Symbol* binSym = bin->evaluateSymbol(ctx);
    std::vector<std::unique_ptr<IRValue>> res;
    auto L = parse(std::move(bin->left), ctx);
    auto R = parse(std::move(bin->right), ctx);

    for (auto& instr : L.code) {
        res.push_back(std::move(instr));
    }

    for (auto& instr : R.code) {
        res.push_back(std::move(instr));
    }

    // TODO: Issue #2
    if (bin->op == "+") {
        if (binSym->type->name->value == "Int") {
            res.push_back(std::make_unique<IRCall>("_BI_Int_add", "_BI_Int", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else if (binSym->type->name->value == "Void") {
            res.push_back(std::make_unique<IRCall>("_BI_Void_add", "_BI_Void", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else {
            res.push_back(std::make_unique<IRCall>(mangleName(binSym->type->mangledName) + "_F3add", mangleName(binSym->type->mangledName), std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        }
    } else if (bin->op == "-") {
        if (binSym->type->name->value == "Int") {
            res.push_back(std::make_unique<IRCall>("_BI_Int_subtract", "_BI_Int", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else if (binSym->type->name->value == "Void") {
            res.push_back(std::make_unique<IRCall>("_BI_Void_subtract", "_BI_Void", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else {
            res.push_back(std::make_unique<IRCall>(mangleName(binSym->type->mangledName) + "_F8subtract", mangleName(binSym->type->mangledName), std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        }
    } else if (bin->op == "*") {
        if (binSym->type->name->value == "Int") {
            res.push_back(std::make_unique<IRCall>("_BI_Int_multiply", "_BI_Int", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else if (binSym->type->name->value == "Void") {
            res.push_back(std::make_unique<IRCall>("_BI_Void_multiply", "_BI_Void", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else {
            res.push_back(std::make_unique<IRCall>(mangleName(binSym->type->mangledName) + "_F8multiply", mangleName(binSym->type->mangledName), std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        }
    } else if (bin->op == "/") {
        if (binSym->type->name->value == "Int") {
            res.push_back(std::make_unique<IRCall>("_BI_Int_divide", "_BI_Int", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else if (binSym->type->name->value == "Void") {
            res.push_back(std::make_unique<IRCall>("_BI_Void_divide", "_BI_Void", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else {
            res.push_back(std::make_unique<IRCall>(mangleName(binSym->type->mangledName) + "_F6divide", mangleName(binSym->type->mangledName), std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        }
    }

    return {};
}

LIRGenerate parseFunction(FunctionDeclaration* func, Context& ctx) {
    auto funcSym = ctx.lookup(static_cast<IdentyfierNode*>(func->name.get())); 
    std::vector<std::unique_ptr<IRValue>> res;
    std::vector<IRValue*> args;

    lastId = 0;

    if (!static_cast<FunctionDeclaration*>(funcSym->node)->isStatic) {
        args.push_back(createArg(mangleName(funcSym->scope->parent->generativeSymbol->mangledName)));
    }

    for (auto& arg : func->parameters) {
        auto argIr = parseArg(std::move(arg), *funcSym->scope);
        args.push_back(argIr);
    }

    std::string typeName;
    if (funcSym->type->name->value == "Int") {
        typeName = "_BI_Int";
    } else if (funcSym->type->name->value == "Void") {
        typeName = "_BI_Void";
    } else {
        typeName = mangleName(funcSym->type->mangledName);
    }

    auto funcIr = std::make_unique<IRFunction>(mangleName(funcSym->mangledName), std::move(args), typeName);
    for (auto& node : func->body) {
        auto ir = parse(std::move(node), *funcSym->scope);
        for (auto& instr : ir.code) {
            funcIr->body.push_back(std::move(instr));
        }
    }

    res.push_back(std::move(funcIr));
    return {
        funcIr.get(),
        std::move(res)
    };
}

LIRGenerate parseClassDeclaration(ClassDeclNode* cls, Context& ctx) {
    auto classSym = ctx.lookup(static_cast<IdentyfierNode*>(cls->name.get()));
    std::vector<std::unique_ptr<IRValue>> body;
    std::vector<std::unique_ptr<IRValue>> structBody;

    lastClassId = 0;

    for (auto& node : cls->members) {
        if (auto var = dynamic_cast<VariableDeclarationNode*>(dynamic_cast<StatementNode*>(node.get())->value.get())) {
            auto varSym = classSym->scope->lookup(dynamic_cast<IdentyfierNode*>(var->name.get()));

            std::string typeName;
            if (varSym->type->name->value == "Int") {
                typeName = "_BI_Int";
            } else if (varSym->type->name->value == "Void") {
                typeName = "_BI_Void";
            } else {
                typeName = mangleName(varSym->type->mangledName);
            }

            auto member = std::make_unique<IRMember>("$" + std::to_string(lastClassId++), typeName);
            structBody.push_back(std::move(member));

            variables[mangleName(varSym->mangledName)] = structBody.back().get();
            continue;
        }

        auto code = parse(std::move(node), *classSym->scope).code;
        for (auto& instr : code) {
            body.push_back(std::move(instr));
        }
    }

    auto stc = std::make_unique<IRStruct>(mangleName(classSym->mangledName), std::move(structBody));
    body.insert(body.begin(), std::move(stc));
    variables[mangleName(classSym->mangledName)] = body[0].get();

    return {
        body[0].get(),
        std::move(body)
    };
}

LIRGenerate parseIdentyfierNode(IdentyfierNode* id, Context& ctx) {
    auto idSym = ctx.lookup(id);
    if (idSym->kind == SymbolKind::CLASS) {
        return { new IRClass(mangleName(idSym->mangledName), idSym->name->value), {} };
    }

    auto it = variables.find(mangleName(idSym->mangledName));
    if (it == variables.end()) {
        throw LIRException(Error(id->position, "Undefined variable: " + id->value));
    }

    auto allocSym = it->second;

    auto var = std::make_unique<IRVariableRead>(allocSym->name, allocSym->type);

    std::vector<std::unique_ptr<IRValue>> res;
    res.push_back(std::move(var));

    return { res.back().get(), std::move(res) };
}

LIRGenerate parseCallNode(CallNode* call, Context& ctx) {
    std::vector<std::unique_ptr<IRValue>> res;
    std::vector<IRValue*> args;

    Symbol* callSym = nullptr;
    if (auto member = dynamic_cast<MemberAccessNode*>(call->callee.get())) {
        auto objectIR = parse(std::move(member->object), ctx);

        for (auto& instr : objectIR.code) {
            res.push_back(std::move(instr));
        }

        auto objectVal = objectIR.mainValue;

        auto classSym = ctx.lookup(objectVal->type);
        callSym = classSym->scope->lookup(static_cast<IdentyfierNode*>(member->member.get()));

        if (!callSym) {
            throw LIRException(Error(call->position, "Unknown method"));
        }

        if (!callSym->isStatic) {
            args.push_back(objectVal);
        }
    } else {
        callSym = ctx.lookup(static_cast<IdentyfierNode*>(call->callee.get()));

        if (callSym->kind == SymbolKind::CLASS) {
            callSym = callSym->scope->lookup("init", PositionSpan(0, 0), false);
        }

        if (!callSym->isStatic) {
            if (ctx.generativeSymbol->isStatic) {
                throw LIRException(Error(call->position, "Cannot call non-static method from static context"));
            }

            args.push_back(new IRVariableRead("%0", mangleName(ctx.parent->generativeSymbol->mangledName)));
        }
    }

    for (auto& arg : call->args) {
        auto argIR = parse(std::move(arg), ctx);

        for (auto& instr : argIR.code) {
            res.push_back(std::move(instr));
        }

        args.push_back(argIR.mainValue);
    }

    if (callSym->kind == SymbolKind::CLASS) {
        auto callIR = std::make_unique<IRCall>(mangleName(callSym->mangledName) + "_F4init", mangleName(callSym->mangledName), args);
        res.push_back(std::move(callIR));
    } else {
        auto callIR = std::make_unique<IRCall>(mangleName(callSym->mangledName), mangleName(callSym->type->mangledName), args);
        res.push_back(std::move(callIR));
    }

    return { res.back().get(), std::move(res) };
}

LIRGenerate parseMemberAccessNode(MemberAccessNode* access, Context& ctx) {
    std::vector<std::unique_ptr<IRValue>> res;

    auto objectIR = parse(std::move(access->object), ctx);

    for (auto& instr : objectIR.code) {
        res.push_back(std::move(instr));
    }

    auto objectVal = objectIR.mainValue;

    auto classSym = ctx.lookup(objectVal->type);
    auto memberSym = classSym->scope->lookup(static_cast<IdentyfierNode*>(access->member.get()));

    auto accessIR = std::make_unique<IRAccess>("%" + std::to_string(lastId++), mangleName(memberSym->type->mangledName), objectVal, mangleName(memberSym->mangledName));
    res.push_back(std::move(accessIR));

    return {
        res.back().get(),
        std::move(res)
    };
}

LIRGenerate parseReturnNode(ReturnNode* ret, Context& ctx) {
    std::vector<std::unique_ptr<IRValue>> res;
    auto valueIr = parse(std::move(ret->value), ctx);

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

LIRGenerate parse(std::unique_ptr<ASTNode> node, Context& ctx) {
    if (auto stmt = dynamic_cast<StatementNode*>(node.get())) {
        return parse(std::move(stmt->value), ctx);
    }

    if (auto num = dynamic_cast<NumberNode*>(node.get())) {
        return parseNumbers(num, ctx);
    } else if (auto decl = dynamic_cast<VariableDeclarationNode*>(node.get())) {
        return parseVariableDeclaration(decl, ctx);
    } else if (auto assign = dynamic_cast<VariableAssigment*>(node.get())) {
        return parseVariableAssigment(assign, ctx);
    } else if (auto bin = dynamic_cast<BinaryNode*>(node.get())) {
        return parseBinaryExpression(bin, ctx);
    } else if (auto func = dynamic_cast<FunctionDeclaration*>(node.get())) {
        return parseFunction(func, ctx);
    } else if (auto cls = dynamic_cast<ClassDeclNode*>(node.get())) {
        return parseClassDeclaration(cls, ctx);
    } else if (auto id = dynamic_cast<IdentyfierNode*>(node.get())) {
        return parseIdentyfierNode(id, ctx);
    } else if (auto call = dynamic_cast<CallNode*>(node.get())) {
        return parseCallNode(call, ctx);
    } else if (auto access = dynamic_cast<MemberAccessNode*>(node.get())) {
        return parseMemberAccessNode(access, ctx);
    } else if (auto ret = dynamic_cast<ReturnNode*>(node.get())) {
        return parseReturnNode(ret, ctx);
    }

    return {};
}

std::vector<std::unique_ptr<IRValue>> generateLIR(std::vector<std::unique_ptr<ASTNode>> nodes, Context& ctx) {
    std::vector<std::unique_ptr<IRValue>> ir;
    for (auto& node : nodes) {
        auto chunk = parse(std::move(node), ctx);

        for (auto& instr : chunk.code) {
            if (instr) {
                ir.push_back(std::move(instr));
            }
        }
    }

    return ir;
}