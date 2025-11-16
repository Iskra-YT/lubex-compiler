#include "parser/ast.hpp"
#include <exception>
#include <iostream>
#include <cmath>
#include "emiter.hpp"

llvm::Value* ASTNode::evaluate() {
    throw std::runtime_error("Internal error: unreachable path");
}

void ASTNode::debug() {
    throw std::runtime_error("Internal error: unreachable path");
}

llvm::Value* StatementNode::evaluate() {
    return value->evaluate();
}

void StatementNode::debug() {
    value->debug();
    std::cout << ";";
}

llvm::Value* NumberNode::evaluate() {
    if (std::floor(value) != value) {
        return llvm::ConstantFP::get(
            llvm::Type::getFloatTy(*emiterContext),
            static_cast<float>(value)
        );
    }
    else {
        return llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(*emiterContext),
            static_cast<int32_t>(value)
        );
    }
}

void NumberNode::debug() {
    std::cout << value;
}

llvm::Value* BinaryNode::evaluate() {
    llvm::Value* L = left->evaluate();
    llvm::Value* R = right->evaluate();

    if (op == "+")
        return emiterBuilder->CreateAdd(L, R, "addtmp");
    else if (op == "-")
        return emiterBuilder->CreateSub(L, R, "subtmp");
    else if (op == "*")
        return emiterBuilder->CreateMul(L, R, "multmp");
    else if (op == "/")
        return emiterBuilder->CreateSDiv(L, R, "divtmp");
    else
        throw std::runtime_error("Internal error: unreachable path");
}

void BinaryNode::debug() {
    std::cout << "(";
    left->debug();
    std::cout << op;
    right->debug();
    std::cout << ")";
}

llvm::Value* VariableDeclarationNode::evaluate() {
    return nullptr;
}

void VariableDeclarationNode::debug() {
    std::cout << "let";
    name->evaluate();
    std::cout << ":";
    type->debug();
    std::cout << "=";
    value->debug();
}

llvm::Value* IdentyfierNode::evaluate() {
    return nullptr;
}

void IdentyfierNode::debug() {
    std::cout << value;
}

llvm::Value* VariableAssigment::evaluate() {
    return nullptr;
}

void VariableAssigment::debug() {
    name->evaluate();
    std::cout << "=";
    value->evaluate();
}

llvm::Value* ArgDeclaration::evaluate() {
    return nullptr;
}

void ArgDeclaration::debug() {
    std::cout << "arg";
    name->evaluate();
    std::cout << ":";
    type->evaluate();
}

llvm::Value* FunctionDeclaration::evaluate() {
    return nullptr;
}

void FunctionDeclaration::debug() {
    std::cout << "func";
    name->evaluate();
    std::cout << "(";
    for (auto& param : parameters) {
        param->evaluate();
    }
    std::cout << ") -> ";
    for (auto& node : body) {
        node->evaluate();
    }
}

llvm::Value* ClassDeclNode::evaluate() {
    return nullptr;
}

void ClassDeclNode::debug() {
    std::cout << "class";
    name->evaluate();
    for (auto& node : members) {
        node->evaluate();
    }
}