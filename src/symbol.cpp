#include "parser/ast.hpp"
#include "evaluator.hpp"

extern IdentyfierNode intType;
extern IdentyfierNode objectType;

bool inStatic = false;

Symbol* ASTNode::evaluateSymbol(Context& ctx) {
    throw std::runtime_error("Internal error: unreachable path");
}

Symbol* StatementNode::evaluateSymbol(Context& ctx) {
    return value->evaluateSymbol(ctx);
}

Symbol* NumberNode::evaluateSymbol(Context& ctx) {
    auto sym = ctx.lookup(&intType);
    sym->type = sym;
    return sym;
}

Symbol* BinaryNode::evaluateSymbol(Context& ctx) {
    if (ctx.phase == PassPhase::TYPE_CHECK) {
        auto L = left->evaluateSymbol(ctx);
        auto R = right->evaluateSymbol(ctx);

        if (L->type->name->value != R->type->name->value) {
            ctx.errors.push_back(Error(position, "Type mismatch in binary expression"));
        }

        return L->type;
    }
    return nullptr;
}


Symbol* VariableDeclarationNode::evaluateSymbol(Context& ctx) {
    if (ctx.phase == PassPhase::TYPE_CHECK) {
        auto t = type->evaluateSymbol(ctx);
        ctx.declare(std::make_unique<Symbol>(SymbolKind::VARIABLE, dynamic_cast<IdentyfierNode*>(name.get()), t, static_cast<ASTNode*>(this)));

        if (value) {
            auto v = value->evaluateSymbol(ctx);
            auto sym = ctx.lookup(static_cast<IdentyfierNode*>(name.get()));
            if (v && t && sym && t->type->name->value != v->type->name->value) {
                ctx.errors.push_back(Error(position, "Type mismatch in variable declaration"));
            }
        }
    }

    return nullptr;
}

Symbol* IdentyfierNode::evaluateSymbol(Context& ctx) {
    return ctx.lookup(this);
}

Symbol* VariableAssigment::evaluateSymbol(Context& ctx) {
    if (ctx.phase == PassPhase::TYPE_CHECK) {
        auto n = name->evaluateSymbol(ctx);
        auto v = value->evaluateSymbol(ctx);

        if (!n || !v) return nullptr;
        if (!n || n->kind != SymbolKind::VARIABLE) {
            ctx.errors.push_back(Error(position, "Cannot assign to non-variable symbol"));
        } else if (n->type->name->value != v->type->name->value) {
            ctx.errors.push_back(Error(position, "Type mismatch in variable assignment"));
        }
    }

    return nullptr;
}

Symbol* ArgDeclaration::evaluateSymbol(Context& ctx) {
    if (ctx.phase == PassPhase::DECLARATION) {
        ctx.declare(std::make_unique<Symbol>(
            SymbolKind::VARIABLE,
            static_cast<IdentyfierNode*>(name.get()),
            nullptr,
            static_cast<ASTNode*>(this)
        ));
    }

    if (ctx.phase == PassPhase::MIDPASS) {
        ctx.declare(std::make_unique<Symbol>(
            SymbolKind::VARIABLE,
            static_cast<IdentyfierNode*>(name.get()),
            type->evaluateSymbol(ctx),
            static_cast<ASTNode*>(this)
        ));
    }

    return nullptr;
}

Symbol* FunctionDeclaration::evaluateSymbol(Context& ctx) {
    if (ctx.symbolKind != SymbolKind::CLASS) {
        ctx.errors.push_back(Error(position, "Function declarations are only allowed inside class declarations"));
        return nullptr;
    }

    if (ctx.phase == PassPhase::DECLARATION) {
        auto fnSym = std::make_unique<Symbol>(
            SymbolKind::FUNCTION,
            static_cast<IdentyfierNode*>(name.get()),
            nullptr,
            static_cast<ASTNode*>(this)
        );

        Context* fnCtx = ctx.addChild();
        fnCtx->symbolKind = SymbolKind::FUNCTION;
        fnCtx->phase = ctx.phase;

        fnSym->isStatic = isStatic ? true : false;

        fnSym->scope = fnCtx;
        fnSym->scope->generativeSymbol = fnSym.get();

        ctx.declare(std::move(fnSym));
    }

    if (ctx.phase == PassPhase::MIDPASS) {
        auto fnSym = ctx.lookup(static_cast<IdentyfierNode*>(name.get()));
        fnSym->type = type->evaluateSymbol(ctx);

        Context* fnCtx = fnSym->scope;
        fnCtx->phase = ctx.phase;

        for (auto& param : parameters) {
            param->evaluateSymbol(*fnCtx);
        }
    }

    if (ctx.phase == PassPhase::TYPE_CHECK) {
        auto fnSym = ctx.lookup(static_cast<IdentyfierNode*>(name.get()));
        Context* fnCtx = fnSym->scope;
        fnCtx->phase = ctx.phase;

        if (isStatic) {
            inStatic = true;
        }

        for (auto& stmt : body) {
            stmt->evaluateSymbol(*fnCtx);
        }
    }

    inStatic = false;
    return nullptr;
}

Symbol* ClassDeclNode::evaluateSymbol(Context& ctx) {
    if (ctx.phase == PassPhase::DECLARATION) {
        auto sym = std::make_unique<Symbol>(
            SymbolKind::CLASS,
            static_cast<IdentyfierNode*>(name.get()),
            ctx.lookup(&objectType),
            static_cast<ASTNode*>(this)
        );

        sym->scope = ctx.addChild();
        sym->scope->symbolKind = SymbolKind::CLASS;
        sym->scope->generativeSymbol = sym.get();
        ctx.declare(std::move(sym));
    }

    auto classSym = ctx.lookup(static_cast<IdentyfierNode*>(name.get()));
    auto classCtx = classSym->scope;

    classCtx->phase = ctx.phase;
    for (auto& member : members) {
        member->evaluateSymbol(*classCtx);
    }

    return nullptr;
}

Symbol* ModuleDeclaration::evaluateSymbol(Context& ctx) {
    if (ctx.phase == PassPhase::DECLARATION) {
        ctx.declare(std::make_unique<Symbol>(
            SymbolKind::MODULE,
            static_cast<IdentyfierNode*>(name.get()),
            nullptr,
            static_cast<ASTNode*>(this)
        ));
    }

    ctx.symbolKind = SymbolKind::MODULE;

    return nullptr;
}

Symbol* CallNode::evaluateSymbol(Context& ctx) {
    auto call = callee->evaluateSymbol(ctx);
    if (!call) return nullptr;

    for (auto& arg : args) {
        arg->evaluateSymbol(ctx);
    }

    if (call->kind != SymbolKind::FUNCTION && call->kind != SymbolKind::CLASS) {
        ctx.errors.push_back(Error(position, "Called object is not a function"));
        return nullptr;
    }

    if (call->kind == SymbolKind::FUNCTION && static_cast<FunctionDeclaration*>(call->node)->parameters.size() > args.size()) { 
        ctx.errors.push_back(Error(position, "Argument count mismatch in function call"));
    }
    
    return call->type;
}

Symbol* MemberAccessNode::evaluateSymbol(Context& ctx) {
    auto obj = object->evaluateSymbol(ctx);
    if (!obj || (obj->kind != SymbolKind::CLASS && obj->type->kind != SymbolKind::CLASS)) {
        ctx.errors.push_back(Error(position, "Object is not a class instance"));
        return nullptr;
    }
    
    auto memberSym = obj->scope ? obj->scope->lookup(
        static_cast<IdentyfierNode*>(member.get())
    ) : obj->type->scope->lookup(
        static_cast<IdentyfierNode*>(member.get())
    );

    if (!memberSym) {
        ctx.errors.push_back(Error(position, "No such class member"));
    }

    if (memberSym && memberSym->kind == SymbolKind::FUNCTION) {
        auto funcDecl = static_cast<FunctionDeclaration*>(memberSym->node);
        if (!funcDecl->isStatic) {
            ctx.errors.push_back(Error(position, "Cannot access non-static member without instance"));
        }
    }

    return memberSym;
}
