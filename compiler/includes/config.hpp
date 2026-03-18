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

struct Options {
    std::string stdPath;
};

struct ProjectConfig {
    std::string name;
    std::string version;
    std::string sourceDir;
    std::string buildDir;
    int optimalization;
    std::vector<Target> targets;
    Options options;
};

ProjectConfig readConfig();

#endif //CONFIG_LUBEX_HPP