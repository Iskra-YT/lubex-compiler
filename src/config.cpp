#include <filesystem>
#include <fstream>
#include "extern/json.hpp"
#include "config.hpp"

ProjectConfig readConfig() {
    const std::filesystem::path configPath = std::filesystem::current_path() / "project.json";
    if (!std::filesystem::exists(configPath)) {
        throw std::runtime_error("Project config file not found!");
    }

    std::ifstream inFile(configPath);
    nlohmann::json j;
    inFile >> j;

    ProjectConfig config;
    config.name = j.value("name", "MyLubexProject");
    config.version = j.value("version", "0.1.0");
    config.sourceDir = j.value("sourceDir", "src");
    config.optimalization = j.value("optimalization", 1);
    config.entrypoint = j.value("entrypoint", "main.HelloWorldProgram.main");

    if (j.contains("targets") && j["targets"].is_array()) {
        for (auto& t : j["targets"]) {
            Target target;
            target.machine = t.value("machine", "linux-x86_64");
            target.outputName = t.value("outputName", "output");
            config.targets.push_back(target);
        }
    }

    return config;
}
