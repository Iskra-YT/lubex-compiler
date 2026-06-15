#include "optimizer_tests.hpp"
#include "optimizer.hpp"

OPTIMIZER_TEST(SimpleAddition) {
    auto left = std::make_unique<NumberNode>(PositionSpan(0, 0), 2);
    auto right = std::make_unique<NumberNode>(PositionSpan(0, 0), 3);
    auto expr = std::make_unique<BinaryNode>(PositionSpan(0, 0), "+", std::move(left), std::move(right));

    auto stmt = std::make_unique<StatementNode>(PositionSpan(0, 0), std::move(expr));
    std::vector<std::unique_ptr<ASTNode>> nodes;
    nodes.push_back(std::move(stmt));

    getOptimization(&nodes);

    ASSERT_EQ(nodes.size(), 1);
    auto resultStmt = dynamic_cast<StatementNode*>(nodes[0].get());
    ASSERT_NE(resultStmt, nullptr);

    auto resultNum = dynamic_cast<NumberNode*>(resultStmt->value.get());
    ASSERT_NE(resultNum, nullptr);
    ASSERT_EQ(resultNum->value, 5.0);
}

OPTIMIZER_TEST(SimpleSubtraction) {
    auto left = std::make_unique<NumberNode>(PositionSpan(0, 0), 10);
    auto right = std::make_unique<NumberNode>(PositionSpan(0, 0), 3);
    auto expr = std::make_unique<BinaryNode>(PositionSpan(0, 0), "-", std::move(left), std::move(right));

    auto stmt = std::make_unique<StatementNode>(PositionSpan(0, 0), std::move(expr));
    std::vector<std::unique_ptr<ASTNode>> nodes;
    nodes.push_back(std::move(stmt));

    getOptimization(&nodes);

    ASSERT_EQ(nodes.size(), 1);
    auto resultStmt = dynamic_cast<StatementNode*>(nodes[0].get());
    ASSERT_NE(resultStmt, nullptr);

    auto resultNum = dynamic_cast<NumberNode*>(resultStmt->value.get());
    ASSERT_NE(resultNum, nullptr);
    ASSERT_EQ(resultNum->value, 7.0);
}

OPTIMIZER_TEST(SimpleMultiplication) {
    auto left = std::make_unique<NumberNode>(PositionSpan(0, 0), 4);
    auto right = std::make_unique<NumberNode>(PositionSpan(0, 0), 5);
    auto expr = std::make_unique<BinaryNode>(PositionSpan(0, 0), "*", std::move(left), std::move(right));

    auto stmt = std::make_unique<StatementNode>(PositionSpan(0, 0), std::move(expr));
    std::vector<std::unique_ptr<ASTNode>> nodes;
    nodes.push_back(std::move(stmt));

    getOptimization(&nodes);

    ASSERT_EQ(nodes.size(), 1);
    auto resultStmt = dynamic_cast<StatementNode*>(nodes[0].get());
    ASSERT_NE(resultStmt, nullptr);

    auto resultNum = dynamic_cast<NumberNode*>(resultStmt->value.get());
    ASSERT_NE(resultNum, nullptr);
    ASSERT_EQ(resultNum->value, 20.0);
}

OPTIMIZER_TEST(SimpleDivision) {
    auto left = std::make_unique<NumberNode>(PositionSpan(0, 0), 20);
    auto right = std::make_unique<NumberNode>(PositionSpan(0, 0), 4);
    auto expr = std::make_unique<BinaryNode>(PositionSpan(0, 0), "/", std::move(left), std::move(right));

    auto stmt = std::make_unique<StatementNode>(PositionSpan(0, 0), std::move(expr));
    std::vector<std::unique_ptr<ASTNode>> nodes;
    nodes.push_back(std::move(stmt));

    getOptimization(&nodes);

    ASSERT_EQ(nodes.size(), 1);
    auto resultStmt = dynamic_cast<StatementNode*>(nodes[0].get());
    ASSERT_NE(resultStmt, nullptr);

    auto resultNum = dynamic_cast<NumberNode*>(resultStmt->value.get());
    ASSERT_NE(resultNum, nullptr);
    ASSERT_EQ(resultNum->value, 5.0);
}

OPTIMIZER_TEST(NestedExpressionFolding) {
    auto left_inner = std::make_unique<NumberNode>(PositionSpan(0, 0), 2);
    auto right_inner = std::make_unique<NumberNode>(PositionSpan(0, 0), 3);
    auto inner_expr = std::make_unique<BinaryNode>(PositionSpan(0, 0), "+", std::move(left_inner), std::move(right_inner));

    auto right_outer = std::make_unique<NumberNode>(PositionSpan(0, 0), 4);
    auto full_expr = std::make_unique<BinaryNode>(PositionSpan(0, 0), "*", std::move(inner_expr), std::move(right_outer));

    auto node = std::make_unique<StatementNode>(PositionSpan(0, 0), std::move(full_expr));
    std::vector<std::unique_ptr<ASTNode>> nodes;
    nodes.push_back(std::move(node));

    getOptimization(&nodes);

    ASSERT_EQ(nodes.size(), 1);
    auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
    ASSERT_NE(stmt, nullptr);

    auto resultNum = dynamic_cast<NumberNode*>(stmt->value.get());
    ASSERT_NE(resultNum, nullptr);
    ASSERT_EQ(resultNum->value, 20.0);
}

OPTIMIZER_TEST(ChainedAddition) {
    auto a = std::make_unique<NumberNode>(PositionSpan(0, 0), 1);
    auto b = std::make_unique<NumberNode>(PositionSpan(0, 0), 2);
    auto ab = std::make_unique<BinaryNode>(PositionSpan(0, 0), "+", std::move(a), std::move(b));

    auto c = std::make_unique<NumberNode>(PositionSpan(0, 0), 3);
    auto abc = std::make_unique<BinaryNode>(PositionSpan(0, 0), "+", std::move(ab), std::move(c));

    auto stmt = std::make_unique<StatementNode>(PositionSpan(0, 0), std::move(abc));
    std::vector<std::unique_ptr<ASTNode>> nodes;
    nodes.push_back(std::move(stmt));

    getOptimization(&nodes);

    ASSERT_EQ(nodes.size(), 1);
    auto resultStmt = dynamic_cast<StatementNode*>(nodes[0].get());
    ASSERT_NE(resultStmt, nullptr);

    auto resultNum = dynamic_cast<NumberNode*>(resultStmt->value.get());
    ASSERT_NE(resultNum, nullptr);
    ASSERT_EQ(resultNum->value, 6.0);
}

OPTIMIZER_TEST(NoOptimizationOnVariables) {
    auto left = std::make_unique<IdentyfierNode>(PositionSpan(0, 0), "x");
    auto right = std::make_unique<NumberNode>(PositionSpan(0, 0), 5);
    auto expr = std::make_unique<BinaryNode>(PositionSpan(0, 0), "+", std::move(left), std::move(right));

    auto stmt = std::make_unique<StatementNode>(PositionSpan(0, 0), std::move(expr));
    std::vector<std::unique_ptr<ASTNode>> nodes;
    nodes.push_back(std::move(stmt));

    getOptimization(&nodes);

    // Expression should remain a BinaryNode since it contains a variable
    ASSERT_EQ(nodes.size(), 1);
    auto resultStmt = dynamic_cast<StatementNode*>(nodes[0].get());
    ASSERT_NE(resultStmt, nullptr);

    auto resultBin = dynamic_cast<BinaryNode*>(resultStmt->value.get());
    ASSERT_NE(resultBin, nullptr);
    ASSERT_EQ(resultBin->op, "+");
}

OPTIMIZER_TEST(AlgebraicSimplification) {
    auto left_inner = std::make_unique<NumberNode>(PositionSpan(0, 0), 2);
    auto right_inner = std::make_unique<NumberNode>(PositionSpan(0, 0), 3);
    auto inner_expr = std::make_unique<BinaryNode>(PositionSpan(0, 0), "+", std::move(left_inner), std::move(right_inner));

    auto right_outer = std::make_unique<NumberNode>(PositionSpan(0, 0), 4);
    auto full_expr = std::make_unique<BinaryNode>(PositionSpan(0, 0), "*", std::move(inner_expr), std::move(right_outer));

    auto node = std::make_unique<StatementNode>(PositionSpan(0, 0), std::move(full_expr));

    std::vector<std::unique_ptr<ASTNode>> nodes;
    nodes.push_back(std::move(node));

    getOptimization(&nodes);

    ASSERT_EQ(nodes.size(), 1);

    auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
    ASSERT_NE(stmt, nullptr);

    auto returnNum = dynamic_cast<NumberNode*>(stmt->value.get());
    ASSERT_NE(returnNum, nullptr);

    ASSERT_EQ(returnNum->value, 20.0);
}