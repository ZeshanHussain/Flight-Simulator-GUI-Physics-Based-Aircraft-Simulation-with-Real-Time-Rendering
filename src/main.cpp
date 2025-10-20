#include "aircraft.hpp"
#include "atmosphere.hpp"
#include "flight_dynamics.hpp"
#include "instruments.hpp"
#include "renderer.hpp"
#include "input_handler.hpp"
#include "audio_system.hpp"
#include "imgui.h"
#include <iostream>
#include <chrono>

int main() {
    // Initialize renderer
    Renderer renderer;
    if (!renderer.initialize(1920, 1080, "6DOF Flight Simulator")) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return -1;
    }
    
    std::cout << "=== 6DOF Flight Simulator ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  W/S or Up/Down    - Elevator (Pitch)" << std::endl;
    std::cout << "  A/D or Left/Right - Aileron (Roll)" << std::endl;
    std::cout << "  Q/E               - Rudder (Yaw)" << std::endl;
    std::cout << "  Z/X or PgUp/PgDn  - Throttle" << std::endl;
    std::cout << "  Space             - Center controls" << std::endl;
    std::cout << "  P                 - Pause/Resume" << std::endl;
    std::cout << "  R                 - Reset" << std::endl;
    std::cout << "  ESC               - Exit" << std::endl;
    std::cout << std::endl;
    
    // Initialize simulation objects
    Aircraft aircraft;
    Atmosphere atmosphere;
    FlightDynamics dynamics(&aircraft, &atmosphere);
    Instruments instruments;
    InputHandler inputHandler;
    
    // Initialize audio system
    AudioSystem audioSystem;
    if (!audioSystem.initialize()) {
        std::cerr << "Warning: Failed to initialize audio system" << std::endl;
        std::cerr << "         Continuing without sound..." << std::endl;
    }
    
    // Timing
    auto lastTime = std::chrono::high_resolution_clock::now();
    const double dt = 1.0 / 60.0;  // 60 Hz simulation
    double accumulator = 0.0;
    
    // Main loop
    while (!renderer.shouldClose()) {
        // Calculate elapsed time
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = currentTime - lastTime;
        lastTime = currentTime;
        
        accumulator += elapsed.count();
        
        // Handle input
        inputHandler.update(renderer.getWindow(), aircraft, dt);
        
        // Check for reset
        if (inputHandler.shouldReset()) {
            dynamics.reset();
            inputHandler.clearReset();
        }
        
        // Fixed timestep update
        while (accumulator >= dt && !inputHandler.isPaused()) {
            dynamics.update(dt);
            accumulator -= dt;
        }
        
        // Update audio system
        const AircraftState& state = aircraft.getState();
        bool isStalling = aircraft.getAirspeed() < 40.0; // Stall speed ~40 m/s
        audioSystem.update(state.throttle, aircraft.getAirspeed(), 
                          aircraft.getAltitude(), isStalling);
        
        // Render
        renderer.beginFrame();
        
        // Show control panel
        ImGui::Begin("Flight Simulator Control", nullptr, 
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
        
        ImGui::Text("6 Degrees of Freedom Flight Simulator");
        ImGui::Separator();
        
        if (inputHandler.isPaused()) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "PAUSED");
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "RUNNING");
        }
        
        ImGui::Separator();
        ImGui::Text("Instructions:");
        ImGui::BulletText("Use W/S for pitch (elevator)");
        ImGui::BulletText("Use A/D for roll (aileron)");
        ImGui::BulletText("Use Q/E for yaw (rudder)");
        ImGui::BulletText("Use Z/X for throttle");
        ImGui::BulletText("Press SPACE to center controls");
        ImGui::BulletText("Press P to pause/resume");
        ImGui::BulletText("Press R to reset");
        
        ImGui::Separator();
        ImGui::Text("Simulation Rate: %.1f Hz", 1.0 / dt);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        
        ImGui::End();
        
        // Render instruments
        instruments.render(aircraft);
        
        // Render 3D view
        renderer.render3DView(aircraft);
        
        // Finish frame
        renderer.endFrame();
        
        // Check for ESC key to exit
        if (glfwGetKey(renderer.getWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            break;
        }
    }
    
    std::cout << "Shutting down..." << std::endl;
    audioSystem.shutdown();
    renderer.shutdown();
    
    return 0;
}

