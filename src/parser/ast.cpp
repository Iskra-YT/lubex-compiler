#include "parser/ast.hpp"
#include <exception>
#include <iostream>
#include <cmath>
#include "emiter.hpp"
#include "evaluator.hpp"

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
    return nullptr;
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
        left->evaluateSymbol(ctx);
        right->evaluateSymbol(ctx);
    }
    return nullptr;
}

void VariableDeclarationNode::debug() {
    std::cout << "let";
    name->debug();
    std::cout << ":";
    type->debug();
    std::cout << "=";
    value->debug();
}

Symbol* VariableDeclarationNode::evaluateSymbol(Context& ctx) {
    if (ctx.phase == PassPhase::TYPE_CHECK) {
        type->evaluateSymbol(ctx);
        value->evaluateSymbol(ctx);
        ctx.declare(std::make_unique<Symbol>(SymbolKind::VARIABLE, dynamic_cast<IdentyfierNode*>(name.get())));
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
        name->evaluateSymbol(ctx);
        value->evaluateSymbol(ctx);
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
            static_cast<IdentyfierNode*>(name.get())
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
    if (ctx.phase == PassPhase::DECLARATION) {
        ctx.declare(std::make_unique<Symbol>(
            SymbolKind::FUNCTION,
            static_cast<IdentyfierNode*>(name.get())
        ));
    }

    if (ctx.phase == PassPhase::TYPE_CHECK) {
        type->evaluateSymbol(ctx);

        Context* fnCtx = ctx.addChild();

        for (auto& param : parameters) {
            param->evaluateSymbol(*fnCtx);
        }

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
        ctx.declare(std::make_unique<Symbol>(
            SymbolKind::CLASS,
            static_cast<IdentyfierNode*>(name.get())
        ));
    }

    Context* classCtx = ctx.addChild();
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
            static_cast<IdentyfierNode*>(name.get())
        ));
    }

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
    callee->evaluateSymbol(ctx);
    for (auto& arg : args) {
        arg->evaluateSymbol(ctx);
    }
    return nullptr;
}

void MemberAccessNode::debug() {
    object->debug();
    std::cout << ".";
    member->debug();
}

Symbol* MemberAccessNode::evaluateSymbol(Context& ctx) {
    object->evaluateSymbol(ctx);
    return nullptr;
}
