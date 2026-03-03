#include <cstring>
#include "init.hpp"
#include "compile.hpp"

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
            return EXIT_SUCCESS;
        }

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}