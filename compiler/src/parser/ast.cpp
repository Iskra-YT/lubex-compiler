#include "parser/ast.hpp"
#include <stdexcept>
#include <iostream>
#include <cmath>
#include "emiter/emiter.hpp"
#include "evaluator.hpp"
#include "debug.hpp"

IdentyfierNode intType(PositionSpan(0, 0), "Number");
IdentyfierNode objectType(PositionSpan(0, 0), "Object");
IdentyfierNode voidType(PositionSpan(0, 0), "Void");
IdentyfierNode stringType(PositionSpan(0, 0), "String");
IdentyfierNode nullType(PositionSpan(0, 0), "Null");

void ASTNode::debug() {
    throw std::runtime_error("Internal error: unreachable path");
}

void StatementNode::debug() {
    value->debug();
    DEBUG_OUTPUT << ";";
}

void NumberNode::debug() {
    DEBUG_OUTPUT << value;
}

void BinaryNode::debug() {
    DEBUG_OUTPUT << "(";
    left->debug();
    DEBUG_OUTPUT << op;
    right->debug();
    DEBUG_OUTPUT << ")";
}

void VariableDeclarationNode::debug() {
    DEBUG_OUTPUT << "let";
    name->debug();
    DEBUG_OUTPUT << ":";
    if (type) type->debug();
    DEBUG_OUTPUT << "=";
    if (value) value->debug();
}

void IdentyfierNode::debug() {
    DEBUG_OUTPUT << value;
}

void VariableAssigment::debug() {
    name->debug();
    DEBUG_OUTPUT << "=";
    value->debug();
}

void ArgDeclaration::debug() {
    DEBUG_OUTPUT << "arg";
    name->debug();
    DEBUG_OUTPUT << ":";
    type->debug();
}

void FunctionDeclaration::debug() {
    DEBUG_OUTPUT << "func";
    name->debug();
    DEBUG_OUTPUT << "(";
    for (auto& param : parameters) {
        param->debug();
    }
    DEBUG_OUTPUT << ") -> ";
    for (auto& node : body) {
        node->debug();
    }
}

void ClassDeclNode::debug() {
    DEBUG_OUTPUT << "class";
    name->debug();
    for (auto& node : members) {
        node->debug();
    }
}

void ModuleDeclaration::debug() {
    DEBUG_OUTPUT << "module";
    name->debug();
}

void CallNode::debug() {
    callee->debug();
    DEBUG_OUTPUT << "(";
    for (auto& arg : args) {
        arg->debug();
    }
    DEBUG_OUTPUT << ")";
}

void MemberAccessNode::debug() {
    DEBUG_OUTPUT << "(";
    object->debug();
    DEBUG_OUTPUT << ".";
    member->debug();
    DEBUG_OUTPUT << ")";
}

void ReturnNode::debug() {
    DEBUG_OUTPUT << "return ";
    if (value) value->debug();
}

void AttributesNode::debug(){
    DEBUG_OUTPUT << "attr value ";
    value->debug();
}

void ImportNode::debug() {
    DEBUG_OUTPUT << "import ";
    value->debug();
}

void StringNode::debug() {
    DEBUG_OUTPUT << "\"" << value << "\"";
}

void ThisNode::debug() {
    DEBUG_OUTPUT << "this";
}

void NullNode::debug() {
    DEBUG_OUTPUT << "null";
}

void NullableTypeNode::debug() {
    baseType->debug();
    DEBUG_OUTPUT << "?";
}

void NullCoalescingNode::debug() {
    left->debug();
    DEBUG_OUTPUT << " ?: ";
    right->debug();
}

void NullCheckNode::debug() {
    value->debug();
    DEBUG_OUTPUT << "??";
}

void SafeNavigationNode::debug() {
    object->debug();
    DEBUG_OUTPUT << "?.";
    member->debug();
}

void UnaryNode::debug() {
    DEBUG_OUTPUT << op;
    value->debug();
}