#!/bin/bash

# Compile script for Flight Simulator with Audio

echo "Compiling 6DOF Flight Simulator with Audio Support..."

g++ -std=c++17 \
    -I./include \
    -I./external/imgui \
    -I./external/imgui/backends \
    -I./external \
    src/*.cpp \
    external/imgui/imgui.cpp \
    external/imgui/imgui_draw.cpp \
    external/imgui/imgui_widgets.cpp \
    external/imgui/imgui_tables.cpp \
    external/imgui/backends/imgui_impl_glfw.cpp \
    external/imgui/backends/imgui_impl_opengl3.cpp \
    -lglfw -lGL -ldl -lpthread -lm \
    -o flight_simulator

if [ $? -eq 0 ]; then
    echo "✓ Compilation successful!"
    echo ""
    echo "Run with: ./flight_simulator"
else
    echo "✗ Compilation failed"
    exit 1
fi

