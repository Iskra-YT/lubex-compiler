#ifndef OPTIMIZER_LUBEX_HPP
#define OPTIMIZER_LUBEX_HPP

#include <iostream>
#include <cstdint>
#include <vector>

#include "parser.hpp"

void getOptimalization(std::vector<std::unique_ptr<ASTNode>>* nodes);

#endif //OPTIMIZER_LUBEX_HPP