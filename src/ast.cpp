#include "ast.hpp"
#include <exception>
#include <iostream>

void ASTNode::evaluate() {
    throw std::runtime_error("Internal error: unreachable path");
}

void StatementNode::evaluate() {
    value->evaluate();
    std::cout << "; ";
}

void NumberNode::evaluate() {
    std::cout << value << " ";
}

void BinaryNode::evaluate() {
    std::cout << "(";
    left->evaluate();
    std::cout << op << " ";
    right->evaluate();
    std::cout << ")";
}