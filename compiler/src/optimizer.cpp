#include "optimizer.hpp"
#include <iostream>

void optimizeNode(std::unique_ptr<ASTNode>& node);

static NumberNode* asNumber(ASTNode* node) {
    return dynamic_cast<NumberNode*>(node);
}

static void replaceBinaryWithNumber(std::unique_ptr<ASTNode>& expr, double value) {
    auto pos = expr->position;
    expr = std::make_unique<NumberNode>(pos, value);
}

static void optimizeBinary(BinaryNode* expr, std::unique_ptr<ASTNode>& node) {
    if (expr->left) optimizeNode(expr->left);
    if (expr->right) optimizeNode(expr->right);

    NumberNode* leftNum = asNumber(expr->left.get());
    NumberNode* rightNum = asNumber(expr->right.get());
    if (!leftNum || !rightNum) return;

    double l = leftNum->value;
    double r = rightNum->value;
    double result = 0.0;
    bool canFold = true;

    if (expr->op == "+") result = l + r;
    else if (expr->op == "-") result = l - r;
    else if (expr->op == "*") result = l * r;
    else if (expr->op == "/") {
        if (r != 0.0) result = l / r;
        else canFold = false;
    } else if (expr->op == "==") {
        result = (l == r) ? 1.0 : 0.0;
    } else if (expr->op == "!=") {
        result = (l != r) ? 1.0 : 0.0;
    } else if (expr->op == "<") {
        result = (l < r) ? 1.0 : 0.0;
    } else if (expr->op == ">") {
        result = (l > r) ? 1.0 : 0.0;
    } else if (expr->op == "<=") {
        result = (l <= r) ? 1.0 : 0.0;
    } else if (expr->op == ">=") {
        result = (l >= r) ? 1.0 : 0.0;
    } else {
        canFold = false;
    }

    if (!canFold) return;

    replaceBinaryWithNumber(node, result);
}

static void optimizeUnary(UnaryNode* expr, std::unique_ptr<ASTNode>& node) {
    if (expr->value) optimizeNode(expr->value);

    NumberNode* valNum = asNumber(expr->value.get());
    if (!valNum) return;

    double v = valNum->value;
    double result = 0.0;
    bool canFold = true;

    if (expr->op == "!") {
        result = (v == 0.0) ? 1.0 : 0.0;
    } else if (expr->op == "~") {
        result = static_cast<double>(~static_cast<int64_t>(v));
    } else {
        canFold = false;
    }

    if (!canFold) return;

    auto pos = expr->position;
    node = std::make_unique<NumberNode>(pos, result);
}

void optimizeNode(std::unique_ptr<ASTNode>& node) {
    if (!node) return;

    if (auto expr = dynamic_cast<BinaryNode*>(node.get())) {
        optimizeBinary(expr, node);
    } else if (auto un = dynamic_cast<UnaryNode*>(node.get())) {
        optimizeUnary(un, node);
    } else if (auto stmt = dynamic_cast<StatementNode*>(node.get())) {
        optimizeNode(stmt->value);
    } else if (auto cls = dynamic_cast<ClassDeclNode*>(node.get())) {
        for (auto& n : cls->members) {
            optimizeNode(n);
        }
    } else if (auto fn = dynamic_cast<FunctionDeclaration*>(node.get())) {
        for (auto& n : fn->body) {
            optimizeNode(n);
        }
    } else if (auto var = dynamic_cast<VariableDeclarationNode*>(node.get())) {
        optimizeNode(var->value);
    } else if (auto asg = dynamic_cast<VariableAssigment*>(node.get())) {
        optimizeNode(asg->value);
    }
}

void getOptimization(std::vector<std::unique_ptr<ASTNode>>* nodes) {
    for (auto& node : *nodes) {
        optimizeNode(node);
    }
}
