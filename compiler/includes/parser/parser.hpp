#ifndef PARSER_LUBEX_HPP
#define PARSER_LUBEX_HPP

#include "lexer.hpp"
#include "error.hpp"
#include "parser/ast.hpp"
#include <functional>
#include <memory>
#include <vector>

struct InstructionStep {
    TokenType expectedType;
    std::string expectedValue;
    std::function<void(Token&, void*)> action;
    bool consumesToken = false;
    int optional = 0;
};

struct InstructionSet {
    std::vector<InstructionStep> steps;
    std::function<std::unique_ptr<ASTNode>(PositionSpan, void*)> finalize;
    bool needsSemicolon = true;
};

enum class ExpectationType {
    EXPECT_TYPE,
    EXPECT_VALUE,
    EXPECT_BOTH,
};

struct ExpectedToken {
    ExpectationType expect;
    TokenType type;
    std::string value;

    ExpectedToken(TokenType type) : expect(ExpectationType::EXPECT_TYPE), type(type) {
    }
    ExpectedToken(std::string value) : expect(ExpectationType::EXPECT_VALUE), value(value) {
    }
    ExpectedToken(TokenType type, std::string value) : expect(ExpectationType::EXPECT_BOTH), type(type), value(value) {
    }
};

class Parser {
    private:
    std::vector<Error> errors;
    void pushError(Error error);

    std::vector<Token> tokens;
    size_t position = 0;

    Token getCurrent();
    Token advance();

    VisibilityKind currentVisibilityLevel = VisibilityKind::PRIVATE;
    inline void setDefaultVisibility() {
        currentVisibilityLevel = VisibilityKind::PRIVATE;
    }

    bool currentOverrideLevel = false;
    inline void setDefaultOverride() {
        currentOverrideLevel = false;
    }

    std::unique_ptr<ASTNode> parsePrimary();
    std::unique_ptr<ASTNode> parseFactor();
    std::unique_ptr<ASTNode> parseTerm();
    std::unique_ptr<ASTNode> parseExpr();
    std::unique_ptr<ASTNode> parseStatement();

    std::vector<std::unique_ptr<ASTNode>> parseFunctionArgs();
    std::vector<std::unique_ptr<ASTNode>> parseBlock();

    std::unique_ptr<ASTNode> parseInstruction(InstructionSet& instrSet, void* context);

    bool allowCallAndMember = true;

    // Instruction Sets
    InstructionSet varDeclInstr;
    InstructionSet funcDeclInstr;
    InstructionSet classDeclInstr;
    InstructionSet moduleDeclInstr;
    InstructionSet constDeclInstr;
    InstructionSet returnDeclInstr;
    InstructionSet attributesInstr;
    InstructionSet includeInstr;

    void initVarDecl();
    void initFuncDecl();
    void initClassDecl();
    void initModuleDecl();
    void initConstDecl();
    void initReturnDecl();
    void initAttributes();
    void initIncludeDecl();

    public:
    Parser(std::vector<Token> toks);
    std::vector<std::unique_ptr<ASTNode>> parse();
    std::vector<Error> getErrors();
};

#endif // PARSER_LUBEX_HPP