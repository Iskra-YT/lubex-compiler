#include <cctype>
#include <filesystem>
#include <fstream>
#include "extern/json.hpp"
#include "init.hpp"

bool createFolder(std::filesystem::path path) {
    try {
        if (!std::filesystem::exists(path)) {
            return std::filesystem::create_directory(path);
        }
        return true;
    } catch(std::exception& e) {
        return false;
    }
}

void createCompilerConfig(const std::filesystem::path& projectDir, const std::string& projectName) {
    nlohmann::json config;

    config["name"] = projectName;
    config["version"] = "1.0.0";
    config["sourceDir"] = "src";
    config["optimalization"] = 1;
    config["entrypoint"] = "main." + std::string(1, std::toupper(projectName[0])) + projectName.substr(1) + ".entry"; // module.class.function

    config["targets"] = nlohmann::json::array();
    nlohmann::json target;
    target["machine"] = "linux-x86_64";
    target["outputName"] = projectName;
    config["targets"].push_back(target);

    std::filesystem::create_directories(projectDir / "src");

    std::ofstream outFile(projectDir / "project.json");
    outFile << config.dump(4);
}

bool initProject(char* name) {
    std::filesystem::path projectPath = std::filesystem::current_path() / name;
    if (!createFolder(projectPath)) return false;
    createCompilerConfig(projectPath, std::string(name));

    // TODO: Create Hello, World program in src
    std::filesystem::path mainSourcePath = projectPath / "src" / "main.lbx";
    std::ofstream out(mainSourcePath);
    if (!out) return false;

    return true;
}