#include "parser/ast.hpp"
#include <stdexcept>
#include <iostream>
#include <cmath>
#include "emiter.hpp"
#include "evaluator.hpp"

static IdentyfierNode intType(PositionSpan(0, 0), "Int");

void ASTNode::debug() {
    throw std::runtime_error("Internal error: unreachable path");
}

Symbol* ASTNode::evaluateSymbol(Context& ctx) {
    throw std::runtime_error("Internal error: unreachable path");
}

void StatementNode::debug() {
    value->debug();
    std::cout << ";";
}

Symbol* StatementNode::evaluateSymbol(Context& ctx) {
    return value->evaluateSymbol(ctx);
}

void NumberNode::debug() {
    std::cout << value;
}

Symbol* NumberNode::evaluateSymbol(Context& ctx) {
    return ctx.lookup(&intType);
}

void BinaryNode::debug() {
    std::cout << "(";
    left->debug();
    std::cout << op;
    right->debug();
    std::cout << ")";
}

Symbol* BinaryNode::evaluateSymbol(Context& ctx) {
    if (ctx.phase == PassPhase::TYPE_CHECK) {
        auto L = left->evaluateSymbol(ctx);
        auto R = right->evaluateSymbol(ctx);

        if (!L || !R || !L->type || !R->type) return nullptr;
        if (L->name->value != R->name->value) {
            ctx.errors.push_back(Error(position, "Type mismatch in binary operation"));
        }

        return L->type;
    }
    return nullptr;
}

void VariableDeclarationNode::debug() {
    std::cout << "let";
    name->debug();
    std::cout << ":";
    type->debug();
    std::cout << "=";
    if(value) value->debug();
}

Symbol* VariableDeclarationNode::evaluateSymbol(Context& ctx) {
    if (ctx.phase == PassPhase::TYPE_CHECK) {
        auto t = type->evaluateSymbol(ctx);
        ctx.declare(std::make_unique<Symbol>(SymbolKind::VARIABLE, dynamic_cast<IdentyfierNode*>(name.get()), t, static_cast<ASTNode*>(this)));

        if (value) {
            auto v = value->evaluateSymbol(ctx);
            auto sym = ctx.lookup(static_cast<IdentyfierNode*>(name.get()));
            if (v && t && sym && t->name->value != v->name->value) {
                ctx.errors.push_back(Error(position, "Type mismatch in variable declaration"));
            }
        }
    }

    return nullptr;
}

void IdentyfierNode::debug() {
    std::cout << value;
}

Symbol* IdentyfierNode::evaluateSymbol(Context& ctx) {
    return ctx.lookup(this);
}

void VariableAssigment::debug() {
    name->debug();
    std::cout << "=";
    value->debug();
}

Symbol* VariableAssigment::evaluateSymbol(Context& ctx) {
    if (ctx.phase == PassPhase::TYPE_CHECK) {
        auto n = name->evaluateSymbol(ctx);
        auto v = value->evaluateSymbol(ctx);

        if (!n || !v) return nullptr;
        if (!n || n->kind != SymbolKind::VARIABLE) {
            ctx.errors.push_back(Error(position, "Cannot assign to non-variable symbol"));
        } else if (n->type->name->value != v->name->value) {
            ctx.errors.push_back(Error(position, "Type mismatch in variable assignment"));
        }
    }

    return nullptr;
}

void ArgDeclaration::debug() {
    std::cout << "arg";
    name->debug();
    std::cout << ":";
    type->debug();
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

void FunctionDeclaration::debug() {
    std::cout << "func";
    name->debug();
    std::cout << "(";
    for (auto& param : parameters) {
        param->debug();
    }
    std::cout << ") -> ";
    for (auto& node : body) {
        node->debug();
    }
}

Symbol* FunctionDeclaration::evaluateSymbol(Context& ctx) {
    if (ctx.symbolKind != SymbolKind::CLASS) {
        ctx.errors.push_back(Error(position, "Function declarations are only allowed inside class declarations"));
        return nullptr;
    }

    if (ctx.phase == PassPhase::DECLARATION) {
        ctx.declare(std::make_unique<Symbol>(
            SymbolKind::FUNCTION,
            static_cast<IdentyfierNode*>(name.get()),
            nullptr,
            static_cast<ASTNode*>(this)
        ));
    }

    if (ctx.phase == PassPhase::MIDPASS) {
        auto fnSym = ctx.lookup(static_cast<IdentyfierNode*>(name.get()));
        fnSym->type = type->evaluateSymbol(ctx);

        Context* fnCtx = ctx.addChild();
        fnCtx->phase = ctx.phase;
        fnCtx->symbolKind = SymbolKind::FUNCTION;

        for (auto& param : parameters) {
            param->evaluateSymbol(*fnCtx);
        }
    }

    if (ctx.phase == PassPhase::TYPE_CHECK) {
        Context* fnCtx = ctx.addChild();
        fnCtx->phase = ctx.phase;
        fnCtx->symbolKind = SymbolKind::FUNCTION;

        for (auto& stmt : body) {
            stmt->evaluateSymbol(*fnCtx);
        }
    }

    return nullptr;
}

void ClassDeclNode::debug() {
    std::cout << "class";
    name->debug();
    for (auto& node : members) {
        node->debug();
    }
}

Symbol* ClassDeclNode::evaluateSymbol(Context& ctx) {
    if (ctx.phase == PassPhase::DECLARATION) {
        auto sym = std::make_unique<Symbol>(
            SymbolKind::CLASS,
            static_cast<IdentyfierNode*>(name.get()),
            nullptr,
            static_cast<ASTNode*>(this)
        );

        sym->scope = ctx.addChild();
        sym->scope->symbolKind = SymbolKind::CLASS;
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

void ModuleDeclaration::debug() {
    std::cout << "module";
    name->debug();
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

void CallNode::debug() {
    callee->debug();
    std::cout << "(";
    for (auto& arg : args) {
        arg->debug();
    }
    std::cout << ")";
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

void MemberAccessNode::debug() {
    object->debug();
    std::cout << ".";
    member->debug();
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
