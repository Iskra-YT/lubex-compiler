#ifndef AST_NODE_HPP
#define AST_NODE_HPP

#include <memory>
#include <string>

#include <llvm/IR/Value.h>

#include "lexer.hpp"

struct ASTNode {
    PositionSpan position;
    ASTNode(PositionSpan pos) : position(pos) {}

    virtual ~ASTNode() = default;
    virtual llvm::Value* evaluate();
};

struct StatementNode : ASTNode {
    std::unique_ptr<ASTNode> value;
    explicit StatementNode(PositionSpan span, std::unique_ptr<ASTNode> val): ASTNode(span),  value(std::move(val)) {}

    llvm::Value* evaluate() override;
};

struct NumberNode : ASTNode {
    double value;
    explicit NumberNode(PositionSpan span, double val): ASTNode(span),  value(val) {}

    llvm::Value* evaluate() override;
};

struct BinaryNode : ASTNode {
    std::string op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;

    BinaryNode(PositionSpan span, std::string o, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : ASTNode(span), op(std::move(o)), left(std::move(l)), right(std::move(r)) {}

    llvm::Value* evaluate() override;
};

#endif // AST_NODE_HPP
