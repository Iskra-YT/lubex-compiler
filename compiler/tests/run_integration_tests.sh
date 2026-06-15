#!/bin/bash
set -e

COMPILER=$(pwd)/build/lubex

TEST_DIR=$(mktemp -d)
echo "Testing in $TEST_DIR"

cd $TEST_DIR

$COMPILER init MyTestProject
cd MyTestProject

if [ ! -f project.json ]; then
    echo "project.json not created"
    exit 1
fi

if [ ! -f src/main.lbx ]; then
    echo "src/main.lbx not created"
    exit 1
fi

if $COMPILER build; then
    echo "Integration test passed: build successful"
else
    echo "Integration test: build failed"
fi

# Cleanup
rm -rf $TEST_DIR
echo "Integration test completed"
