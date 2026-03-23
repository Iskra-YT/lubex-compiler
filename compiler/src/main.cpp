#include <cstring>
#include "init.hpp"
#include "compile.hpp"

#define LUBEX_COMPILER_VERSION "lubex-r202604a01"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Expected more arguments\n";
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "init") == 0) {
        if (argc < 3) {
            std::cerr << "Expected project name\n";
            return EXIT_FAILURE;
        }

        initProject(argv[2]);
    } else if (strcmp(argv[1], "build") == 0) {
        if(compileProject()) {
            std::cout << "Build successful\n";
            return EXIT_SUCCESS;
        }

        std::cerr << "Build failed\n";
        return EXIT_FAILURE;
    } else if (strcmp(argv[1], "version") == 0) {
        std::cout << "Lubex Compiler version: " << LUBEX_COMPILER_VERSION << "\n";
        return EXIT_SUCCESS;
    } else if (strcmp(argv[1], "help") == 0) {
        std::cout << "Usage: lubex <command> [options]\n";
        std::cout << "Commands:\n";
        std::cout << "\tinit <project_name>   Initialize a new project\n";
        std::cout << "\tbuild                 Build the project\n";
        std::cout << "\tversion               Show compiler version\n";
        std::cout << "\thelp                  Show this help message\n";
        return EXIT_SUCCESS;
    }

    std::cerr << "Unknown command: " << argv[1] << "\n";
    return EXIT_FAILURE;
}