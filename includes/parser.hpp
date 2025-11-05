#ifndef PARSER_LUBEX_HPP
#define PARSER_LUBEX_HPP

#include <vector>
#include "lexer.hpp"
#include "error.hpp"
#include "ast.hpp"

enum class ExpectationType {
    EXPECT_TYPE,
    EXPECT_VALUE,
    EXPECT_BOTH,
};

struct ExpectedToken {
    ExpectationType expect;
    TokenType type;
    std::string value;

    ExpectedToken(TokenType type): expect(ExpectationType::EXPECT_TYPE), type(type) {}
    ExpectedToken(std::string value): expect(ExpectationType::EXPECT_VALUE), value(value) {}
    ExpectedToken(TokenType type, std::string value): expect(ExpectationType::EXPECT_BOTH), type(type), value(value) {}
};

class Parser {
    private:
        std::vector<Error> errors;
        void pushError(Error error);

        std::vector<Token> tokens;
        size_t position = 0;

        Token getCurrent();
        Token advance();
        bool eat(ExpectedToken expectation);

        std::unique_ptr<ASTNode> parseFactor();
        std::unique_ptr<ASTNode> parseTerm();
        std::unique_ptr<ASTNode> parseExpr();
        std::unique_ptr<ASTNode> parseStatement();
    public:
        Parser(std::vector<Token> toks): tokens(toks) {}
        std::vector<std::unique_ptr<ASTNode>> parse();
        std::vector<Error> getErrors();
};

#endif // PARSER_LUBEX_HPP