#include "parser/parser.hpp"
#include <exception>
#include "parser/context.hpp"

Parser::Parser(std::vector<Token> toks): tokens(toks) {
    initVarDecl();
}

Token Parser::getCurrent() {
    if (position < tokens.size()) return tokens[position];
    return Token("", TokenType::EOF_TOKEN);
}

Token Parser::advance() {
    if (position < tokens.size()) position++;
    return getCurrent();
}

void Parser::pushError(Error error) {
    while (position < tokens.size() && !getCurrent().match(Token(";", TokenType::DELIMITER_TOKEN))) {
        advance();
    }

    if (getCurrent().match(Token(";", TokenType::DELIMITER_TOKEN))) {
        advance();
    }

    errors.push_back(error);
}

std::vector<Error> Parser::getErrors() {
    return errors;
}

std::vector<std::unique_ptr<ASTNode>> Parser::parse() {
    std::vector<std::unique_ptr<ASTNode>> nodes;
    while (position < tokens.size() && getCurrent().type != TokenType::EOF_TOKEN) {
        nodes.push_back(parseStatement());
    }

    return nodes;
}

std::unique_ptr<ASTNode> Parser::parseInstruction(InstructionSet& instrSet, void* context) {
    Position start = getCurrent().position.start;
    for (auto& step : instrSet.steps) {
        Token tok = getCurrent();

        if (step.expectedType != TokenType::ANY) {
            if (tok.type != step.expectedType ||
                (!step.expectedValue.empty() && tok.value != step.expectedValue)) {
                pushError(Error(tok.position, "Unexpected token: " + tok.value));
                return nullptr;
            }
        }

        step.action(tok, context);

        if (!step.consumesToken) {
            advance();
        }
    }

    return instrSet.finalize(PositionSpan(start, getCurrent().position.end), context);
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    Token tok = getCurrent();

    if (tok.match(Token("let", TokenType::KEYWORD_TOKEN))) {
        VarDeclContext ctx;
        return parseInstruction(varDeclInstr, &ctx);
    }

    auto node = parseExpr();
    if (!getCurrent().match(Token(";", TokenType::DELIMITER_TOKEN))) {
        pushError(Error(PositionSpan(node->position.start, getCurrent().position.end), "Expected ';' after statement"));
        return nullptr;
    }
    advance();
    return std::make_unique<StatementNode>(PositionSpan(node->position.start, getCurrent().position.end), std::move(node));
}

std::unique_ptr<ASTNode> Parser::parseExpr() {
    auto node = parseTerm();

    while (true) {
        Token tok = getCurrent();
        if (tok.match(Token("+", TokenType::ARITHMETIC_TOKEN)) || tok.match(Token("-", TokenType::ARITHMETIC_TOKEN))) {
            std::string op = tok.value;
            advance();
            auto right = parseTerm();
            node = std::make_unique<BinaryNode>(PositionSpan(tok.position.start, getCurrent().position.end), op, std::move(node), std::move(right));
        } else {
            break;
        }
    }

    return node;
}

std::unique_ptr<ASTNode> Parser::parseTerm() {
    auto node = parseFactor();

    while (true) {
        Token tok = getCurrent();

        if (tok.match(Token("*", TokenType::ARITHMETIC_TOKEN)) ||
            tok.match(Token("/", TokenType::ARITHMETIC_TOKEN))) {
            std::string op = tok.value;
            advance();
            auto right = parseFactor();
            node = std::make_unique<BinaryNode>(PositionSpan(tok.position.start, getCurrent().position.end), op, std::move(node), std::move(right));
        } else {
            break;
        }
    }

    return node;
}

std::unique_ptr<ASTNode> Parser::parseFactor() {
    Token tok = getCurrent();

    if (tok.type == TokenType::NUMBER_TOKEN) {
        advance();
        return std::make_unique<NumberNode>(PositionSpan(tok.position.start, getCurrent().position.end), std::stod(tok.value));
    }

    if (tok.match(Token("(", TokenType::DELIMITER_TOKEN))) {
        advance();
        auto node = parseExpr();
        if (!getCurrent().match(Token(")", TokenType::DELIMITER_TOKEN))) {
            pushError(Error(tok.position, "Expected ')'"));
            return nullptr;
        }
        advance();
        return node;
    }

    if (tok.match(Token("-", TokenType::ARITHMETIC_TOKEN))) {
        advance();
        auto right = parseFactor();
        auto zero = std::make_unique<NumberNode>(PositionSpan(0, 0), 0.0);
        return std::make_unique<BinaryNode>(PositionSpan(tok.position.start, getCurrent().position.end), "-", std::move(zero), std::move(right));
    }

    if (tok.match(Token("+", TokenType::ARITHMETIC_TOKEN))) {
        advance();
        return parseFactor();
    }

    if (tok.type == TokenType::IDENTYFIER_TOKEN) {
        advance();
        return std::make_unique<IdentyfierNode>(tok.position, tok.value);
    }

    pushError(Error(tok.position, "Unexpected token in factor: " + tok.value));
    advance();
    return nullptr;
}