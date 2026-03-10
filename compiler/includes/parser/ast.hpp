#ifndef AST_NODE_HPP
#define AST_NODE_HPP

#include <memory>
#include <string>

#include <llvm/IR/Value.h>

#include "lexer.hpp"

enum class VisibilityKind {
    PRIVATE,
    INTERNAL,
    PUBLIC
};

struct Symbol;
struct Context;

struct ASTNode {
    PositionSpan position;
    ASTNode(PositionSpan pos) : position(pos) {}

    virtual ~ASTNode() = default;
    virtual Symbol* evaluateSymbol(Context& ctx);
    virtual void debug();
};

struct StatementNode : ASTNode {
    std::unique_ptr<ASTNode> value;
    explicit StatementNode(PositionSpan span, std::unique_ptr<ASTNode> val): ASTNode(span),  value(std::move(val)) {}

    Symbol* evaluateSymbol(Context& ctx) override;
    void debug() override;
};

struct NumberNode : ASTNode {
    double value;
    explicit NumberNode(PositionSpan span, double val): ASTNode(span),  value(val) {}

    Symbol* evaluateSymbol(Context& ctx) override;
    void debug() override;
};

struct VariableDeclarationNode : ASTNode {
    std::unique_ptr<ASTNode> name;
    std::unique_ptr<ASTNode> value;
    std::unique_ptr<ASTNode> type;
    bool isConst;

    VariableDeclarationNode(PositionSpan span, std::unique_ptr<ASTNode> name, std::unique_ptr<ASTNode> value, std::unique_ptr<ASTNode> type, bool isConst): ASTNode(span), name(std::move(name)), value(std::move(value)), type(std::move(type)), isConst(isConst) {}

    Symbol* evaluateSymbol(Context& ctx) override;
    void debug() override;
};

struct IdentyfierNode : ASTNode {
    std::string value;
    explicit IdentyfierNode(PositionSpan span, std::string val): ASTNode(span),  value(val) {}

    Symbol* evaluateSymbol(Context& ctx) override;
    void debug() override;
};

struct VariableAssigment : ASTNode {
    std::unique_ptr<ASTNode> name;
    std::unique_ptr<ASTNode> value;

    VariableAssigment(PositionSpan span, std::unique_ptr<ASTNode> name, std::unique_ptr<ASTNode> value) : ASTNode(span), name(std::move(name)), value(std::move(value)) {}
    
    Symbol* evaluateSymbol(Context& ctx) override;
    void debug() override;
};

struct ArgDeclaration : ASTNode {
    std::unique_ptr<ASTNode> name;
    std::unique_ptr<ASTNode> type;

    ArgDeclaration(PositionSpan span, std::unique_ptr<ASTNode> name, std::unique_ptr<ASTNode> type) : ASTNode(span), name(std::move(name)), type(std::move(type)) {}
    
    Symbol* evaluateSymbol(Context& ctx) override;
    void debug() override;
};

struct FunctionDeclaration : ASTNode {
    std::unique_ptr<ASTNode> name;
    std::unique_ptr<ASTNode> type;
    std::vector<std::unique_ptr<ASTNode>> body;
    std::vector<std::unique_ptr<ASTNode>> parameters;
    VisibilityKind visibility;
    bool isForward;
    bool isStatic;

    FunctionDeclaration(PositionSpan span, std::unique_ptr<ASTNode> name, std::unique_ptr<ASTNode> type, std::vector<std::unique_ptr<ASTNode>> parameters, std::vector<std::unique_ptr<ASTNode>> body, bool isForward, bool isStatic, VisibilityKind visibility) : ASTNode(span), name(std::move(name)), type(std::move(type)), body(std::move(body)), parameters(std::move(parameters)), isForward(isForward), isStatic(isStatic), visibility(visibility) {}

    Symbol* evaluateSymbol(Context& ctx) override;
    void debug() override;
};

struct ClassDeclNode : ASTNode {
    std::unique_ptr<ASTNode> name;
    std::vector<std::unique_ptr<ASTNode>> members;
    VisibilityKind visibility;
    bool isForward;

    ClassDeclNode(PositionSpan span,
                  std::unique_ptr<ASTNode> name,
                  std::vector<std::unique_ptr<ASTNode>> members, bool isForward, VisibilityKind visibility)
        : ASTNode(span), name(std::move(name)), members(std::move(members)), isForward(isForward), visibility(visibility) {}

    Symbol* evaluateSymbol(Context& ctx) override;
    void debug() override;
};

struct ModuleDeclaration : ASTNode {
    std::unique_ptr<ASTNode> name;

    ModuleDeclaration(PositionSpan span, std::unique_ptr<ASTNode> name): ASTNode(span), name(std::move(name)) {}
    
    Symbol* evaluateSymbol(Context& ctx) override;
    void debug() override;
};

struct BinaryNode : ASTNode {
    std::string op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;

    BinaryNode(PositionSpan span, std::string o, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : ASTNode(span), op(std::move(o)), left(std::move(l)), right(std::move(r)) {}

    Symbol* evaluateSymbol(Context& ctx) override;
    void debug() override;
};

struct CallNode : ASTNode {
    std::unique_ptr<ASTNode> callee;
    std::vector<std::unique_ptr<ASTNode>> args;

    CallNode(PositionSpan span, std::unique_ptr<ASTNode> c, std::vector<std::unique_ptr<ASTNode>> a) : ASTNode(span), callee(std::move(c)), args(std::move(a)) {}
    
    Symbol* evaluateSymbol(Context& ctx) override;
    void debug() override;
};

struct MemberAccessNode : ASTNode {
    std::unique_ptr<ASTNode> object;
    std::unique_ptr<ASTNode> member;

    MemberAccessNode(PositionSpan span, std::unique_ptr<ASTNode> o, std::unique_ptr<ASTNode> m) : ASTNode(span), object(std::move(o)), member(std::move(m)) {}

    Symbol* evaluateSymbol(Context& ctx) override;
    void debug() override;
};

struct ReturnNode : ASTNode {
    std::unique_ptr<ASTNode> value;

    ReturnNode(PositionSpan span, std::unique_ptr<ASTNode> value) : ASTNode(span), value(std::move(value)) {}
    Symbol* evaluateSymbol(Context& ctx) override;
    void debug() override;
};

struct AttributesNode : ASTNode {
    std::unique_ptr<ASTNode> name;
    std::vector<std::unique_ptr<ASTNode>> params;
    std::unique_ptr<ASTNode> value;

    AttributesNode(PositionSpan span, std::unique_ptr<ASTNode> name, std::vector<std::unique_ptr<ASTNode>> params, std::unique_ptr<ASTNode> value) : ASTNode(span), name(std::move(name)), params(std::move(params)), value(std::move(value)) {}
    Symbol* evaluateSymbol(Context& ctx) override;
    void debug() override;
};

#endif // AST_NODE_HPP
