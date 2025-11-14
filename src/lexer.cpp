#include "lexer.hpp"
#include <stdexcept>

bool isVariableAllowed(char c) {
    if ((c >= 'a' && c <= 'z') || 
        (c >= 'A' && c <= 'Z') || 
        c == '_' || c == '@' || c == '#' || c == '$') {
        return true;
    }
    return false;
}

std::string tokenTypeToString(TokenType type) {
    switch(type) {
        case TokenType::EOF_TOKEN: return "EOF_TOKEN";
        case TokenType::ERR_TOKEN: return "ERR_TOKEN";
        case TokenType::NUMBER_TOKEN: return "NUMBER_TOKEN";
        default: return "UNKNOWN";
    }
}

void Lexer::advance() {
    if (!input.empty()) {
        currentToken = input.front();
        input.erase(input.begin());

        if (currentToken == '\n') {
            currentPosition.line++;
            currentPosition.column = 0;
        } else {
            currentPosition.column++;
        }
        currentPosition.index++;
    } else {
        currentToken = '\0';
    }
}

std::vector<Token> Lexer::lex() {
    std::vector<Token> tokens;
    currentPosition = {1, 0, 0};
    advance();

    while (currentToken != '\0') {
        if (std::isspace(currentToken)) {
            advance();
            continue;
        }

        Position startPos = currentPosition;

        if (std::isdigit(currentToken)) {
            std::string value;
            while (std::isdigit(currentToken)) {
                value += currentToken;
                advance();
            }
            tokens.push_back({{startPos, currentPosition}, value, TokenType::NUMBER_TOKEN});
            continue;
        } else if (isVariableAllowed(currentToken))  {
            std::string value;
            while (isVariableAllowed(currentToken) || std::isdigit(currentToken)) {
                value += currentToken;
                advance();
            }

            if (keywords.find(value) != keywords.end()) tokens.push_back({{startPos, currentPosition}, value, TokenType::KEYWORD_TOKEN});
            else tokens.push_back({{startPos, currentPosition}, value, TokenType::IDENTYFIER_TOKEN});

            continue;
        } else if (currentToken == '+' || currentToken == '-' || currentToken == '*' || currentToken == '/') {
            tokens.push_back({{startPos, currentPosition}, std::string(1, currentToken), TokenType::ARITHMETIC_TOKEN});
            advance();
            continue;
        } else if (currentToken == '(' || currentToken == ')' || currentToken == ';' || currentToken == ':') {
            tokens.push_back({{startPos, currentPosition}, std::string(1, currentToken), TokenType::DELIMITER_TOKEN});
            advance();
            continue;
        } else if (currentToken == '=') {
            tokens.push_back({{startPos, currentPosition}, std::string(1, currentToken), TokenType::ASSIGNMENT_TOKEN});
            advance();
            continue;
        }

        std::string unknown(1, currentToken);
        Position endPos = currentPosition;
        tokens.push_back({{startPos, endPos}, unknown, TokenType::ERR_TOKEN});
        advance();
    }

    tokens.push_back({{currentPosition, currentPosition}, "", TokenType::EOF_TOKEN});
    return tokens;
}