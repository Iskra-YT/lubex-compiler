#ifndef CONFIG_LUBEX_HPP
#define CONFIG_LUBEX_HPP

#include <iostream>
#include <cstdint>
#include <vector>

struct Target {
    std::string entrypoint;
    std::string machine;
    std::string outputName;
};

struct ProjectConfig {
    std::string name;
    std::string version;
    std::string sourceDir;
    int optimalization;
    std::vector<Target> targets;
};

ProjectConfig readConfig();

#endif //CONFIG_LUBEX_HPP