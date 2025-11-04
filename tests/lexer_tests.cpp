#include <gtest/gtest.h>
#include "lexer.hpp"
#include "lexer_tests.hpp"

LEXER_TEST(IntNumbers) {
    std::string input = "123 456 1 2 098 112";
    std::vector<char> in(input.begin(), input.end());

    Lexer lexer(in);
    std::vector<Token> tokens = lexer.lex();

    ASSERT_EQ(tokens.size(), 7);

    ASSERT_EQ(tokens[0].type, TokenType::NUMBER_TOKEN);
    ASSERT_EQ(tokens[0].value, "123");

    ASSERT_EQ(tokens[1].type, TokenType::NUMBER_TOKEN);
    ASSERT_EQ(tokens[1].value, "456");

    ASSERT_EQ(tokens[2].type, TokenType::NUMBER_TOKEN);
    ASSERT_EQ(tokens[2].value, "1");

    ASSERT_EQ(tokens[3].type, TokenType::NUMBER_TOKEN);
    ASSERT_EQ(tokens[3].value, "2");

    ASSERT_EQ(tokens[4].type, TokenType::NUMBER_TOKEN);
    ASSERT_EQ(tokens[4].value, "098");

    ASSERT_EQ(tokens[5].type, TokenType::NUMBER_TOKEN);
    ASSERT_EQ(tokens[5].value, "112");

    ASSERT_EQ(tokens[6].type, TokenType::EOF_TOKEN);
}