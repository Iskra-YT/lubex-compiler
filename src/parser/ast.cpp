#include "parser/ast.hpp"
#include <stdexcept>
#include <iostream>
#include <cmath>
#include "emiter.hpp"
#include "evaluator.hpp"

IdentyfierNode intType(PositionSpan(0, 0), "Int");
IdentyfierNode objectType(PositionSpan(0, 0), "Object");

void ASTNode::debug() {
    throw std::runtime_error("Internal error: unreachable path");
}

void StatementNode::debug() {
    value->debug();
    std::cout << ";";
}

void NumberNode::debug() {
    std::cout << value;
}

void BinaryNode::debug() {
    std::cout << "(";
    left->debug();
    std::cout << op;
    right->debug();
    std::cout << ")";
}

void VariableDeclarationNode::debug() {
    std::cout << "let";
    name->debug();
    std::cout << ":";
    type->debug();
    std::cout << "=";
    if(value) value->debug();
}

void IdentyfierNode::debug() {
    std::cout << value;
}

void VariableAssigment::debug() {
    name->debug();
    std::cout << "=";
    value->debug();
}

void ArgDeclaration::debug() {
    std::cout << "arg";
    name->debug();
    std::cout << ":";
    type->debug();
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

void ClassDeclNode::debug() {
    std::cout << "class";
    name->debug();
    for (auto& node : members) {
        node->debug();
    }
}

void ModuleDeclaration::debug() {
    std::cout << "module";
    name->debug();
}

void CallNode::debug() {
    callee->debug();
    std::cout << "(";
    for (auto& arg : args) {
        arg->debug();
    }
    std::cout << ")";
}

void MemberAccessNode::debug() {
    object->debug();
    std::cout << ".";
    member->debug();
}
