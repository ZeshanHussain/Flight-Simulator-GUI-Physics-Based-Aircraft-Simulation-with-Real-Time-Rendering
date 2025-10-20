#pragma once
#include "aircraft.hpp"
#include <GLFW/glfw3.h>

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    bool initialize(int width, int height, const char* title);
    void shutdown();
    
    bool shouldClose();
    void beginFrame();
    void endFrame();
    
    // Render 3D view
    void render3DView(const Aircraft& aircraft);
    
    GLFWwindow* getWindow() { return window; }
    
private:
    GLFWwindow* window;
    
    void setupOpenGL();
    void drawHorizon(double roll, double pitch);
    void drawAircraft();
    void drawGround(double altitude);
    void drawCompass(double heading);
};

