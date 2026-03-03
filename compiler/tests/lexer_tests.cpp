#include "lexer.hpp"
#include "lexer_tests.hpp"

LEXER_TEST(IntegerLiterals) {
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

LEXER_TEST(ArithmeticSymbols) {
    std::string input = "+- */";
    std::vector<char> in(input.begin(), input.end());

    Lexer lexer(in);
    std::vector<Token> tokens = lexer.lex();

    ASSERT_EQ(tokens.size(), 5);

    ASSERT_EQ(tokens[0].type, TokenType::ARITHMETIC_TOKEN);
    ASSERT_EQ(tokens[0].value, "+");

    ASSERT_EQ(tokens[1].type, TokenType::ARITHMETIC_TOKEN);
    ASSERT_EQ(tokens[1].value, "-");

    ASSERT_EQ(tokens[2].type, TokenType::ARITHMETIC_TOKEN);
    ASSERT_EQ(tokens[2].value, "*");

    ASSERT_EQ(tokens[3].type, TokenType::ARITHMETIC_TOKEN);
    ASSERT_EQ(tokens[3].value, "/");

    ASSERT_EQ(tokens[4].type, TokenType::EOF_TOKEN);
}

LEXER_TEST(Delimiters) {
    std::string input = "( ) : ; -> { } , .";
    std::vector<char> in(input.begin(), input.end());

    Lexer lexer(in);
    std::vector<Token> tokens = lexer.lex();

    ASSERT_EQ(tokens.size(), 10);

    ASSERT_EQ(tokens[0].type, TokenType::DELIMITER_TOKEN);
    ASSERT_EQ(tokens[0].value, "(");

    ASSERT_EQ(tokens[1].type, TokenType::DELIMITER_TOKEN);
    ASSERT_EQ(tokens[1].value, ")");

    ASSERT_EQ(tokens[2].type, TokenType::DELIMITER_TOKEN);
    ASSERT_EQ(tokens[2].value, ":");

    ASSERT_EQ(tokens[3].type, TokenType::DELIMITER_TOKEN);
    ASSERT_EQ(tokens[3].value, ";");

    ASSERT_EQ(tokens[4].type, TokenType::DELIMITER_TOKEN);
    ASSERT_EQ(tokens[4].value, "->");

    ASSERT_EQ(tokens[5].type, TokenType::DELIMITER_TOKEN);
    ASSERT_EQ(tokens[5].value, "{");

    ASSERT_EQ(tokens[6].type, TokenType::DELIMITER_TOKEN);
    ASSERT_EQ(tokens[6].value, "}");

    ASSERT_EQ(tokens[7].type, TokenType::DELIMITER_TOKEN);
    ASSERT_EQ(tokens[7].value, ",");

    ASSERT_EQ(tokens[8].type, TokenType::DELIMITER_TOKEN);
    ASSERT_EQ(tokens[8].value, ".");

    ASSERT_EQ(tokens[9].type, TokenType::EOF_TOKEN);
}

LEXER_TEST(Identyfier) {
    std::string input = "i32 a test _and $e @qwerty";
    std::vector<char> in(input.begin(), input.end());

    Lexer lexer(in);
    std::vector<Token> tokens = lexer.lex();

    ASSERT_EQ(tokens.size(), 7);

    ASSERT_EQ(tokens[0].type, TokenType::IDENTYFIER_TOKEN);
    ASSERT_EQ(tokens[0].value, "i32");

    ASSERT_EQ(tokens[1].type, TokenType::IDENTYFIER_TOKEN);
    ASSERT_EQ(tokens[1].value, "a");

    ASSERT_EQ(tokens[2].type, TokenType::IDENTYFIER_TOKEN);
    ASSERT_EQ(tokens[2].value, "test");

    ASSERT_EQ(tokens[3].type, TokenType::IDENTYFIER_TOKEN);
    ASSERT_EQ(tokens[3].value, "_and");

    ASSERT_EQ(tokens[4].type, TokenType::IDENTYFIER_TOKEN);
    ASSERT_EQ(tokens[4].value, "$e");

    ASSERT_EQ(tokens[5].type, TokenType::IDENTYFIER_TOKEN);
    ASSERT_EQ(tokens[5].value, "@qwerty");

    ASSERT_EQ(tokens[6].type, TokenType::EOF_TOKEN);
}

LEXER_TEST(Keywords) {
    std::string input = "let const class func module";
    std::vector<char> in(input.begin(), input.end());

    Lexer lexer(in);
    std::vector<Token> tokens = lexer.lex();

    ASSERT_EQ(tokens.size(), 6);

    ASSERT_EQ(tokens[0].type, TokenType::KEYWORD_TOKEN);
    ASSERT_EQ(tokens[0].value, "let");

    ASSERT_EQ(tokens[1].type, TokenType::KEYWORD_TOKEN);
    ASSERT_EQ(tokens[1].value, "const");

    ASSERT_EQ(tokens[2].type, TokenType::KEYWORD_TOKEN);
    ASSERT_EQ(tokens[2].value, "class");

    ASSERT_EQ(tokens[3].type, TokenType::KEYWORD_TOKEN);
    ASSERT_EQ(tokens[3].value, "func");

    ASSERT_EQ(tokens[4].type, TokenType::KEYWORD_TOKEN);
    ASSERT_EQ(tokens[4].value, "module");

    ASSERT_EQ(tokens[5].type, TokenType::EOF_TOKEN);
}