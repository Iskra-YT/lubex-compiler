#include "parser.hpp"
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