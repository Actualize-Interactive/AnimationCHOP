# Build script for AnimationCHOP
# Run this script from the root of the project

# Create build directory if it doesn't exist
if (-not (Test-Path -Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
    Write-Host "Created build directory" -ForegroundColor Green
}

# Change to build directory
Set-Location -Path "build"

# Configure CMake
Write-Host "Configuring CMake..." -ForegroundColor Cyan
cmake ..

# Build the project
Write-Host "Building project..." -ForegroundColor Cyan
cmake --build . --config Release

# Go back to the project root
Set-Location -Path ".."

Write-Host "Build completed!" -ForegroundColor Green
