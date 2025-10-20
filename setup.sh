#!/bin/bash

# 6DOF Flight Simulator Setup Script
# This script sets up the development environment and builds the project

set -e  # Exit on error

echo "==================================="
echo "6DOF Flight Simulator Setup"
echo "==================================="
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
else
    OS="unknown"
fi

echo -e "${GREEN}Detected OS: $OS${NC}"
echo ""

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check for required tools
echo "Checking for required tools..."

if ! command_exists cmake; then
    echo -e "${RED}✗ CMake not found${NC}"
    echo "Please install CMake 3.15 or higher"
    exit 1
else
    echo -e "${GREEN}✓ CMake found${NC}"
fi

if ! command_exists g++ && ! command_exists clang++; then
    echo -e "${RED}✗ C++ compiler not found${NC}"
    echo "Please install GCC or Clang"
    exit 1
else
    echo -e "${GREEN}✓ C++ compiler found${NC}"
fi

if ! command_exists git; then
    echo -e "${RED}✗ Git not found${NC}"
    echo "Please install Git"
    exit 1
else
    echo -e "${GREEN}✓ Git found${NC}"
fi

echo ""

# Install system dependencies
echo "Checking system dependencies..."

if [[ "$OS" == "linux" ]]; then
    if command_exists pacman; then
        echo "Detected Arch Linux / Manjaro"
        echo "Install dependencies with:"
        echo "  sudo pacman -S cmake gcc glfw-x11 mesa"
    elif command_exists apt; then
        echo "Detected Ubuntu / Debian"
        echo "Install dependencies with:"
        echo "  sudo apt install build-essential cmake libglfw3-dev libgl1-mesa-dev"
    fi
elif [[ "$OS" == "macos" ]]; then
    if command_exists brew; then
        echo "Installing dependencies with Homebrew..."
        brew install cmake glfw
    else
        echo "Please install Homebrew first: https://brew.sh"
        exit 1
    fi
fi

echo ""

# Clone ImGui if not present
echo "Setting up Dear ImGui..."

if [ ! -d "external/imgui" ]; then
    echo "Cloning Dear ImGui..."
    mkdir -p external
    cd external
    git clone https://github.com/ocornut/imgui.git
    cd imgui
    git checkout v1.89.9
    cd ../..
    echo -e "${GREEN}✓ ImGui cloned successfully${NC}"
else
    echo -e "${GREEN}✓ ImGui already present${NC}"
fi

echo ""

# Build project
echo "Building project..."

# Create build directory
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure with CMake
echo "Running CMake..."
cmake .. || {
    echo -e "${RED}✗ CMake configuration failed${NC}"
    exit 1
}

# Build
echo "Compiling..."
cmake --build . -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) || {
    echo -e "${RED}✗ Build failed${NC}"
    exit 1
}

cd ..

echo ""
echo -e "${GREEN}==================================="
echo "✓ Build successful!"
echo "===================================${NC}"
echo ""
echo "To run the simulator:"
echo "  cd build"
echo "  ./flight_simulator"
echo ""
echo "Controls:"
echo "  W/S or Up/Down    - Elevator (Pitch)"
echo "  A/D or Left/Right - Aileron (Roll)"
echo "  Q/E               - Rudder (Yaw)"
echo "  Z/X or PgUp/PgDn  - Throttle"
echo "  Space             - Center controls"
echo "  P                 - Pause/Resume"
echo "  R                 - Reset"
echo "  ESC               - Exit"
echo ""
echo "Happy Flying! ✈️"

