#!/bin/bash
set -e

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}=== Building and Running Runtime Tests ===${NC}"
mkdir -p runtime/build
cd runtime/build
cmake ..
make runtime_tests
./runtime_tests
cd ../..

echo -e "\n${GREEN}=== Building and Running Compiler Unit Tests ===${NC}"
cd compiler
./build.sh
./test.sh

echo -e "\n${GREEN}=== Running Compiler Integration Tests ===${NC}"
chmod +x tests/run_integration_tests.sh
./tests/run_integration_tests.sh
cd ..

echo -e "\n${GREEN}=== All tests passed successfully! ===${NC}"
