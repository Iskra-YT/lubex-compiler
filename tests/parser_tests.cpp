#include "parser/parser.hpp"
#include "lexer.hpp"
#include "parser_tests.hpp"

PARSER_TEST(BinaryExpression) {
    std::vector<Token> tokens = {
        Token("1", TokenType::NUMBER_TOKEN),
        Token("+", TokenType::ARITHMETIC_TOKEN),
        Token("2", TokenType::NUMBER_TOKEN),
        Token("*", TokenType::ARITHMETIC_TOKEN),
        Token("3", TokenType::NUMBER_TOKEN),
        Token(";", TokenType::DELIMITER_TOKEN),
        Token("", TokenType::EOF_TOKEN)
    };

    Parser parser(tokens);
    auto nodes = parser.parse();

    auto errors = parser.getErrors();
    ASSERT_TRUE(errors.empty());

    ASSERT_EQ(nodes.size(), 1);

    auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
    ASSERT_NE(stmt, nullptr);

    auto expr = dynamic_cast<BinaryNode*>(stmt->value.get());
    ASSERT_NE(expr, nullptr);
    ASSERT_EQ(expr->op, "+");

    auto rightMul = dynamic_cast<BinaryNode*>(expr->right.get());
    ASSERT_NE(rightMul, nullptr);

    ASSERT_EQ(rightMul->op, "*");

    auto leftNum = dynamic_cast<NumberNode*>(expr->left.get());
    auto leftMulNum = dynamic_cast<NumberNode*>(rightMul->left.get());
    auto rightMulNum = dynamic_cast<NumberNode*>(rightMul->right.get());

    ASSERT_NE(leftNum, nullptr);
    ASSERT_NE(leftMulNum, nullptr);
    ASSERT_NE(rightMulNum, nullptr);

    ASSERT_EQ(leftNum->value, 1.0);
    ASSERT_EQ(leftMulNum->value, 2.0);
    ASSERT_EQ(rightMulNum->value, 3.0);
}

PARSER_TEST(VariableDeclaration) {
    {
        std::vector<Token> tokens = {
            Token("let", TokenType::KEYWORD_TOKEN),
            Token("a", TokenType::IDENTYFIER_TOKEN),
            Token(":", TokenType::DELIMITER_TOKEN),
            Token("i32", TokenType::IDENTYFIER_TOKEN),
            Token("=", TokenType::ASSIGNMENT_TOKEN),
            Token("2", TokenType::NUMBER_TOKEN),
            Token(";", TokenType::DELIMITER_TOKEN),
            Token("", TokenType::EOF_TOKEN)
        };

        Parser parser(tokens);
        auto nodes = parser.parse();

        auto errors = parser.getErrors();
        ASSERT_TRUE(errors.empty());
        ASSERT_EQ(nodes.size(), 1);

        auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
        ASSERT_NE(stmt, nullptr);

        auto expr = dynamic_cast<VariableDeclarationNode*>(stmt->value.get());
        ASSERT_NE(expr, nullptr);

        auto name = dynamic_cast<IdentyfierNode*>(expr->name.get());
        ASSERT_NE(name, nullptr);
        ASSERT_EQ(name->value, "a");

        auto value = dynamic_cast<NumberNode*>(expr->value.get());
        ASSERT_NE(value, nullptr);
        ASSERT_EQ(value->value, 2.0);

        auto type = dynamic_cast<IdentyfierNode*>(expr->type.get());
        ASSERT_NE(type, nullptr);
        ASSERT_EQ(type->value, "i32");
    }
    {
        std::vector<Token> tokens = {
            Token("let", TokenType::KEYWORD_TOKEN),
            Token("b", TokenType::IDENTYFIER_TOKEN),
            Token(":", TokenType::DELIMITER_TOKEN),
            Token("i32", TokenType::IDENTYFIER_TOKEN),
            Token(";", TokenType::DELIMITER_TOKEN),
            Token("", TokenType::EOF_TOKEN)
        };

        Parser parser(tokens);
        auto nodes = parser.parse();

        auto errors = parser.getErrors();
        ASSERT_TRUE(errors.empty());
        ASSERT_EQ(nodes.size(), 1);

        auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
        ASSERT_NE(stmt, nullptr);

        auto expr = dynamic_cast<VariableDeclarationNode*>(stmt->value.get());
        ASSERT_NE(expr, nullptr);

        auto name = dynamic_cast<IdentyfierNode*>(expr->name.get());
        ASSERT_NE(name, nullptr);
        ASSERT_EQ(name->value, "b");

        ASSERT_EQ(expr->value.get(), nullptr);

        auto type = dynamic_cast<IdentyfierNode*>(expr->type.get());
        ASSERT_NE(type, nullptr);
        ASSERT_EQ(type->value, "i32");
    }
}

PARSER_TEST(VariableAssigment) {
    std::vector<Token> tokens = {
        Token("a", TokenType::IDENTYFIER_TOKEN),
        Token("=", TokenType::ASSIGNMENT_TOKEN),
        Token("15", TokenType::NUMBER_TOKEN),
        Token(";", TokenType::DELIMITER_TOKEN),
        Token("", TokenType::EOF_TOKEN)
    };

    Parser parser(tokens);
    auto nodes = parser.parse();

    auto errors = parser.getErrors();
    ASSERT_TRUE(errors.empty());

    ASSERT_EQ(nodes.size(), 1);

    auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
    ASSERT_NE(stmt, nullptr);

    auto expr = dynamic_cast<VariableAssigment*>(stmt->value.get());
    ASSERT_NE(expr, nullptr);

    auto name = dynamic_cast<IdentyfierNode*>(expr->name.get());
    ASSERT_NE(name, nullptr);
    ASSERT_EQ(name->value, "a");

    auto value = dynamic_cast<NumberNode*>(expr->value.get());
    ASSERT_NE(value, nullptr);
    ASSERT_EQ(value->value, 15.0);
}

PARSER_TEST(FunctionDeclaration) {
    std::vector<Token> tokens = {
        Token("func", TokenType::KEYWORD_TOKEN),
        Token("a", TokenType::IDENTYFIER_TOKEN),
        Token("(", TokenType::DELIMITER_TOKEN),
        Token("arg", TokenType::KEYWORD_TOKEN),
        Token("b", TokenType::IDENTYFIER_TOKEN),
        Token(":", TokenType::DELIMITER_TOKEN),
        Token("i32", TokenType::IDENTYFIER_TOKEN),
        Token(")", TokenType::DELIMITER_TOKEN),
        Token(":", TokenType::DELIMITER_TOKEN),
        Token("i32", TokenType::IDENTYFIER_TOKEN),
        Token("->", TokenType::DELIMITER_TOKEN),
        Token("{", TokenType::DELIMITER_TOKEN),
        Token("}", TokenType::DELIMITER_TOKEN),
        Token(";", TokenType::DELIMITER_TOKEN),
        Token("", TokenType::EOF_TOKEN)
    };

    Parser parser(tokens);
    auto nodes = parser.parse();

    auto errors = parser.getErrors();
    ASSERT_TRUE(errors.empty());

    ASSERT_EQ(nodes.size(), 1);

    auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
    ASSERT_NE(stmt, nullptr);

    auto func = dynamic_cast<FunctionDeclaration*>(stmt->value.get());
    ASSERT_NE(func, nullptr);

    auto name = dynamic_cast<IdentyfierNode*>(func->name.get());
    ASSERT_NE(name, nullptr);
    ASSERT_EQ(name->value, "a");

    auto retType = dynamic_cast<IdentyfierNode*>(func->type.get());
    ASSERT_NE(retType, nullptr);
    ASSERT_EQ(retType->value, "i32");

    ASSERT_EQ(func->parameters.size(), 1);

    auto arg0 = dynamic_cast<ArgDeclaration*>(func->parameters[0].get());
    ASSERT_NE(arg0, nullptr);

    auto argName = dynamic_cast<IdentyfierNode*>(arg0->name.get());
    ASSERT_NE(argName, nullptr);
    ASSERT_EQ(argName->value, "b");

    auto argType = dynamic_cast<IdentyfierNode*>(arg0->type.get());
    ASSERT_NE(argType, nullptr);
    ASSERT_EQ(argType->value, "i32");

    ASSERT_EQ(func->body.size(), 0);
}