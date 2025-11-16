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
    virtual void debug();
};

struct StatementNode : ASTNode {
    std::unique_ptr<ASTNode> value;
    explicit StatementNode(PositionSpan span, std::unique_ptr<ASTNode> val): ASTNode(span),  value(std::move(val)) {}

    llvm::Value* evaluate() override;
    void debug() override;
};

struct NumberNode : ASTNode {
    double value;
    explicit NumberNode(PositionSpan span, double val): ASTNode(span),  value(val) {}

    llvm::Value* evaluate() override;
    void debug() override;
};

struct VariableDeclarationNode : ASTNode {
    std::unique_ptr<ASTNode> name;
    std::unique_ptr<ASTNode> value;
    std::unique_ptr<ASTNode> type;

    VariableDeclarationNode(PositionSpan span, std::unique_ptr<ASTNode> name, std::unique_ptr<ASTNode> value, std::unique_ptr<ASTNode> type): ASTNode(span), name(std::move(name)), value(std::move(value)), type(std::move(type)) {}

    llvm::Value* evaluate() override;
    void debug() override;
};

struct IdentyfierNode : ASTNode {
    std::string value;
    explicit IdentyfierNode(PositionSpan span, std::string val): ASTNode(span),  value(val) {}

    llvm::Value* evaluate() override;
    void debug() override;
};

struct VariableAssigment : ASTNode {
    std::unique_ptr<ASTNode> name;
    std::unique_ptr<ASTNode> value;

    VariableAssigment(PositionSpan span, std::unique_ptr<ASTNode> name, std::unique_ptr<ASTNode> value) : ASTNode(span), name(std::move(name)), value(std::move(value)) {}
    llvm::Value* evaluate() override;
    void debug() override;
};

struct ArgDeclaration : ASTNode {
    std::unique_ptr<ASTNode> name;
    std::unique_ptr<ASTNode> type;

    ArgDeclaration(PositionSpan span, std::unique_ptr<ASTNode> name, std::unique_ptr<ASTNode> type) : ASTNode(span), name(std::move(name)), type(std::move(type)) {}
    llvm::Value* evaluate() override;
    void debug() override;
};

struct FunctionDeclaration : ASTNode {
    std::unique_ptr<ASTNode> name;
    std::unique_ptr<ASTNode> type;
    std::vector<std::unique_ptr<ASTNode>> body;
    std::vector<std::unique_ptr<ASTNode>> parameters;
    bool isForward;

    FunctionDeclaration(PositionSpan span, std::unique_ptr<ASTNode> name, std::unique_ptr<ASTNode> type, std::vector<std::unique_ptr<ASTNode>> parameters, std::vector<std::unique_ptr<ASTNode>> body, bool isForward) : ASTNode(span), name(std::move(name)), type(std::move(type)), body(std::move(body)), parameters(std::move(parameters)), isForward(isForward) {}
    llvm::Value* evaluate() override;
    void debug() override;
};

struct ClassDeclNode : ASTNode {
    std::unique_ptr<ASTNode> name;
    std::vector<std::unique_ptr<ASTNode>> members;
    bool isForward;

    ClassDeclNode(PositionSpan span,
                  std::unique_ptr<ASTNode> name,
                  std::vector<std::unique_ptr<ASTNode>> members, bool isForward)
        : ASTNode(span), name(std::move(name)), members(std::move(members)), isForward(isForward) {}

    llvm::Value* evaluate() override;
    void debug() override;
};


struct BinaryNode : ASTNode {
    std::string op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;

    BinaryNode(PositionSpan span, std::string o, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : ASTNode(span), op(std::move(o)), left(std::move(l)), right(std::move(r)) {}

    llvm::Value* evaluate() override;
    void debug() override;
};

#endif // AST_NODE_HPP
