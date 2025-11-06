#include "optimizer_tests.hpp"
#include "optimizer.hpp"

OPTIMIZER_TEST(AlgebraicSimplification) {
    auto left_inner = std::make_unique<NumberNode>(PositionSpan(0, 0), 2);
    auto right_inner = std::make_unique<NumberNode>(PositionSpan(0, 0), 3);
    auto inner_expr = std::make_unique<BinaryNode>(PositionSpan(0, 0), "+", std::move(left_inner), std::move(right_inner));

    auto right_outer = std::make_unique<NumberNode>(PositionSpan(0, 0), 4);
    auto full_expr = std::make_unique<BinaryNode>(PositionSpan(0, 0), "*", std::move(inner_expr), std::move(right_outer));

    auto node = std::make_unique<StatementNode>(PositionSpan(0, 0), std::move(full_expr));

    std::vector<std::unique_ptr<ASTNode>> nodes;
    nodes.push_back(std::move(node));

    getOptimalization(&nodes);

    ASSERT_EQ(nodes.size(), 1);

    auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
    ASSERT_NE(stmt, nullptr);

    auto returnNum = dynamic_cast<NumberNode*>(stmt->value.get());
    ASSERT_NE(returnNum, nullptr);

    ASSERT_EQ(returnNum->value, 20.0);
}