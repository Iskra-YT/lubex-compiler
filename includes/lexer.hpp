#ifndef LEXER_LUBEX_HPP
#define LEXER_LUBEX_HPP

#include <cstdint>
#include <string>
#include <vector>

enum class TokenType {
    EOF_TOKEN,
    ERR_TOKEN,
    NUMBER_TOKEN,
    ARITHMETIC_TOKEN,
};

std::string tokenTypeToString(TokenType type);

struct Position {
    size_t line;
    size_t index;
    size_t column;
};

struct PositionSpan {
    Position start;
    Position end;
};

struct Token {
    PositionSpan position;
    std::string value;
    TokenType type;
};

class Lexer {
    private:
        Position currentPosition;
        char currentToken;

        std::vector<char> input;
        void advance();
    
    public:
        Lexer(std::vector<char> file): input(std::move(file)) {}
        std::vector<Token> lex();
};

#endif //LEXER_LUBEX_HPP