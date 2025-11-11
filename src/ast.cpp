#include "ast.hpp"
#include <exception>
#include <iostream>
#include <cmath>
#include "emiter.hpp"

llvm::Value* ASTNode::evaluate() {
    throw std::runtime_error("Internal error: unreachable path");
}

llvm::Value* StatementNode::evaluate() {
    return value->evaluate();
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