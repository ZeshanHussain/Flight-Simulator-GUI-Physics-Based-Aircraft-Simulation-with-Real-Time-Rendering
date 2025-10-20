#pragma once
#include <GLFW/glfw3.h>
#include "aircraft.hpp"

class InputHandler {
public:
    InputHandler();
    
    void update(GLFWwindow* window, Aircraft& aircraft, double dt);
    
    bool isPaused() const { return paused; }
    void togglePause() { paused = !paused; }
    
    bool shouldReset() const { return resetRequested; }
    void clearReset() { resetRequested = false; }
    
private:
    bool paused;
    bool resetRequested;
    
    // Control state
    double elevatorInput;
    double aileronInput;
    double rudderInput;
    double throttleInput;
    
    // Keyboard state tracking
    bool keyStates[GLFW_KEY_LAST];
    
    // Control sensitivity
    static constexpr double CONTROL_RATE = 2.0;      // units/sec
    static constexpr double THROTTLE_RATE = 0.5;     // units/sec
};

