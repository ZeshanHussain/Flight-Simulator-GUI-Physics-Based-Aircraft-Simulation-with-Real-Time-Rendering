#include "input_handler.hpp"
#include <cstring>
#include <algorithm>

InputHandler::InputHandler() 
    : paused(false), resetRequested(false),
      elevatorInput(0.0), aileronInput(0.0), rudderInput(0.0), throttleInput(0.5) {
    std::memset(keyStates, 0, sizeof(keyStates));
}

void InputHandler::update(GLFWwindow* window, Aircraft& aircraft, double dt) {
    AircraftState& state = aircraft.getState();
    
    // Check for pause toggle
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !keyStates[GLFW_KEY_P]) {
        togglePause();
    }
    keyStates[GLFW_KEY_P] = (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS);
    
    // Check for reset
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !keyStates[GLFW_KEY_R]) {
        resetRequested = true;
    }
    keyStates[GLFW_KEY_R] = (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS);
    
    if (paused) return;
    
    // Elevator control (pitch) - W/S or Up/Down
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || 
        glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        elevatorInput += CONTROL_RATE * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || 
        glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        elevatorInput -= CONTROL_RATE * dt;
    }
    
    // Aileron control (roll) - A/D or Left/Right
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || 
        glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        aileronInput -= CONTROL_RATE * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || 
        glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        aileronInput += CONTROL_RATE * dt;
    }
    
    // Rudder control (yaw) - Q/E
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        rudderInput -= CONTROL_RATE * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        rudderInput += CONTROL_RATE * dt;
    }
    
    // Throttle control - Z/X or PageUp/PageDown
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS || 
        glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
        throttleInput += THROTTLE_RATE * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS || 
        glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
        throttleInput -= THROTTLE_RATE * dt;
    }
    
    // Center controls - Space
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        elevatorInput *= 0.95;
        aileronInput *= 0.95;
        rudderInput *= 0.95;
    }
    
    // Apply deadzone and return to center when no input
    auto applyDeadzone = [dt](double& value) {
        if (std::abs(value) < 0.01) value = 0.0;
        value = std::max(-1.0, std::min(1.0, value));
        // Gradual return to center
        value *= 0.98;
    };
    
    applyDeadzone(elevatorInput);
    applyDeadzone(aileronInput);
    applyDeadzone(rudderInput);
    
    // Clamp throttle
    throttleInput = std::max(0.0, std::min(1.0, throttleInput));
    
    // Apply to aircraft
    state.elevator = elevatorInput;
    state.aileron = aileronInput;
    state.rudder = rudderInput;
    state.throttle = throttleInput;
}

