#include "LIR/lir.hpp"
#include "emiter.hpp"

int lastId = 0;
static std::unordered_map<std::string, IRValue*> variables;

LIRGenerate parse(std::unique_ptr<ASTNode> node, Context& ctx) {
    if (auto stmt = dynamic_cast<StatementNode*>(node.get())) {
        return parse(std::move(stmt->value), ctx);
    }

    if (auto num = dynamic_cast<NumberNode*>(node.get())) {
        auto n = std::make_unique<IRNumber>("%" + std::to_string(lastId++), "double", num->value);

        std::vector<IRValue*> args;
        args.push_back(n.get());
        auto call = std::make_unique<IRCall>("_BI_Int_init", "_BI_Int", args);

        std::vector<std::unique_ptr<IRValue>> code;
        code.push_back(std::move(n));
        code.push_back(std::move(call));
        return {code.back().get(), std::move(code)};
    } else if (auto decl = dynamic_cast<VariableDeclarationNode*>(node.get())) {
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
    } else if (auto assign = dynamic_cast<VariableAssigment*>(node.get())) {
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
    } else if (auto bin = dynamic_cast<BinaryNode*>(node.get())) {
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
    } else if (auto func = dynamic_cast<FunctionDeclaration*>(node.get())) {
        // TODO: Add support for isForward and isStatic functions
        auto funcSym = ctx.lookup(static_cast<IdentyfierNode*>(func->name.get())); 
        std::vector<std::unique_ptr<IRValue>> res;
        std::vector<IRValue*> args;

        lastId = 0;

        for (auto& arg : func->parameters) {
            auto argIr = parse(std::move(arg), *funcSym->scope);
            args.push_back(argIr.mainValue);
            for (auto& instr : argIr.code) {
                res.push_back(std::move(instr));
            }
        }

        auto funcIr = std::make_unique<IRFunction>(mangleName(funcSym->mangledName), args, funcSym->type->name->value == "Int" ? "_BI_Int" : mangleName(funcSym->type->mangledName));
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
    } else if (auto cls = dynamic_cast<ClassDeclNode*>(node.get())) {
        //TODO: Add support for isForward classes & IRStruct generation
        auto classSym = ctx.lookup(static_cast<IdentyfierNode*>(cls->name.get()));
        std::vector<std::unique_ptr<IRValue>> body;

        for (auto& node : cls->members) {
            auto code = parse(std::move(node), *classSym->scope).code;
            for (auto& instr : code) {
                body.push_back(std::move(instr));
            }
        }

        return {
            nullptr,
            std::move(body)
        };
    } else if (auto id = dynamic_cast<IdentyfierNode*>(node.get())) {
        auto idSym = ctx.lookup(id);
        auto allocSym = variables[mangleName(idSym->mangledName)];

        auto var = std::make_unique<IRVariableRead>(allocSym->name, allocSym->type);
        std::vector<std::unique_ptr<IRValue>> res;
        res.push_back(std::move(var));
        return {
            res.back().get(),
            std::move(res)
        };
    }

    //TODO: Add function call & member access node

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