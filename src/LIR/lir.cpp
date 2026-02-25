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
    auto alloca = std::make_unique<IRAlloca>("%" + std::to_string(lastId++), varSym->type->name->value == "Int" ? "_BI_Int" : mangleName(varSym->type->mangledName));
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
    Symbol* type = bin->evaluateSymbol(ctx);
    std::vector<std::unique_ptr<IRValue>> res;
    auto L = parse(std::move(bin->left), ctx);
    auto R = parse(std::move(bin->right), ctx);

    for (auto& instr : L.code) {
        res.push_back(std::move(instr));
    }

    for (auto& instr : R.code) {
        res.push_back(std::move(instr));
    }

    // TODO: Add more operators
    if (bin->op == "+") {
        if (type->name->value == "Int") {
            res.push_back(std::make_unique<IRCall>("_BI_Int_add", "_BI_Int", std::vector<IRValue*>{L.mainValue, R.mainValue}));
            return {res.back().get(), std::move(res)};
        } else {
            res.push_back(std::make_unique<IRCall>(mangleName(type->mangledName) + "_F3add", mangleName(type->mangledName), std::vector<IRValue*>{L.mainValue, R.mainValue}));
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

    auto funcIr = std::make_unique<IRFunction>(mangleName(funcSym->mangledName), std::move(args), funcSym->type->name->value == "Int" ? "_BI_Int" : mangleName(funcSym->type->mangledName));
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
            auto member = std::make_unique<IRMember>("$" + std::to_string(lastClassId++), varSym->type->name->value == "Int" ? "_BI_Int" : mangleName(varSym->type->mangledName));
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

    return {
        body[0].get(),
        std::move(body)
    };
}

LIRGenerate parseIdentyfierNode(IdentyfierNode* id, Context& ctx) {
    auto idSym = ctx.lookup(id);
    auto allocSym = variables[mangleName(idSym->mangledName)];

    if (allocSym == nullptr) {
        throw LIRException(Error(id->position, "Undefined variable: " + id->value));
    }

    auto var = std::make_unique<IRVariableRead>(allocSym->name, allocSym->type);
    std::vector<std::unique_ptr<IRValue>> res;
    res.push_back(std::move(var));
    return {
        res.back().get(),
        std::move(res)
    };
}

LIRGenerate parseCallNode(CallNode* call, Context& ctx) {
    auto callSym = ctx.lookup(static_cast<IdentyfierNode*>(call->callee.get()));
    std::vector<std::unique_ptr<IRValue>> res;
    std::vector<IRValue*> args;

    if (!callSym->isStatic && !ctx.generativeSymbol->isStatic) {
        args.push_back(new IRVariableRead("%0", mangleName(ctx.parent->generativeSymbol->mangledName)));
    } else if (!callSym->isStatic) {
        throw LIRException(Error(call->position, "Cannot call non-static method from static context"));
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
    }

    //TODO: Add member access node

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