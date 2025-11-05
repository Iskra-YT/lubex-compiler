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
    DELIMITER_TOKEN,
};

std::string tokenTypeToString(TokenType type);

struct Position {
    size_t line;
    size_t index;
    size_t column;

    Position(int index): index(index), column(0), line(0) {}
    Position(int line, int index, int column): index(index), column(column), line(line) {}
};

struct PositionSpan {
    Position start;
    Position end;

    PositionSpan(Position start, Position end) : start(start), end(end) {}
};

struct Token {
    PositionSpan position;
    std::string value;
    TokenType type;

    Token(std::string value, TokenType type): position(0, 0), value(value), type(type) {}
    Token(PositionSpan span, std::string value, TokenType type): position(span), value(value), type(type) {}

    bool match(Token token) {
        if (token.value == value && token.type == type) {
            return true;
        }
        return false;
    }
};

class Lexer {
    private:
        Position currentPosition;
        char currentToken;

        std::vector<char> input;
        void advance();
    
    public:
        Lexer(std::vector<char> file): input(std::move(file)), currentPosition(0) {}
        std::vector<Token> lex();
};

#endif //LEXER_LUBEX_HPP