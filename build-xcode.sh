#!/bin/bash

# Store the current location to restore it later
originalLocation=$(pwd)

# Function to restore location on exit
cleanup() {
    cd "$originalLocation"
}

# Set trap to ensure we always return to the original directory
trap cleanup EXIT

# Create build directory if it doesn't exist
if [ ! -d "build-xcode" ]; then
    mkdir -p build-xcode
    echo -e "\033[32mCreated build directory\033[0m"
fi

# Configure CMake
echo -e "\033[36mConfiguring CMake...\033[0m"
cd build-xcode
cmake -G Xcode ..

# # Build the project
# echo -e "\033[36mBuilding project...\033[0m"

# # Release is the default build configuration
# # If an argument is provided, use it as the build configuration
# buildConfig=${1:-Release}
# echo -e "\033[36mBuilding with configuration: $buildConfig\033[0m"
# cmake --build . --config "$buildConfig"

# echo -e "\033[32mBuild completed!\033[0m"
