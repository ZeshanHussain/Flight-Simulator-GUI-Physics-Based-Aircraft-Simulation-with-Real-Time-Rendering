#include "renderer.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <cmath>
#include <cstdio>
#include <iostream>

Renderer::Renderer() : window(nullptr) {}

Renderer::~Renderer() {
    shutdown();
}

bool Renderer::initialize(int width, int height, const char* title) {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    // OpenGL 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create window
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    
    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui::StyleColorsDark();
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    setupOpenGL();
    
    return true;
}

void Renderer::shutdown() {
    if (window) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        
        glfwDestroyWindow(window);
        glfwTerminate();
        window = nullptr;
    }
}

bool Renderer::shouldClose() {
    return window && glfwWindowShouldClose(window);
}

void Renderer::beginFrame() {
    glfwPollEvents();
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Renderer::endFrame() {
    ImGui::Render();
    
    int displayW, displayH;
    glfwGetFramebufferSize(window, &displayW, &displayH);
    glViewport(0, 0, displayW, displayH);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    glfwSwapBuffers(window);
}

void Renderer::setupOpenGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::render3DView(const Aircraft& aircraft) {
    const AircraftState& state = aircraft.getState();
    
    ImGui::Begin("3D View", nullptr, ImGuiWindowFlags_NoCollapse);
    
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    ImVec2 windowPos = ImGui::GetCursorScreenPos();
    ImVec2 center(windowPos.x + windowSize.x * 0.5f, 
                  windowPos.y + windowSize.y * 0.5f);
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    // Draw horizon
    drawHorizon(state.roll, state.pitch);
    
    // Draw ground reference
    drawGround(aircraft.getAltitude());
    
    // Draw compass
    drawCompass(state.yaw * 180.0 / M_PI);
    
    // Info overlay
    ImGui::SetCursorPos(ImVec2(10, 30));
    ImGui::BeginChild("3DInfo", ImVec2(250, 120), true, ImGuiWindowFlags_NoScrollbar);
    ImGui::Text("3D VISUALIZATION");
    ImGui::Separator();
    ImGui::Text("Altitude: %.0f ft", aircraft.getAltitude() * 3.28084);
    ImGui::Text("Airspeed: %.0f kts", aircraft.getAirspeed() * 1.94384);
    ImGui::Text("Heading: %.0f°", state.yaw * 180.0 / M_PI);
    ImGui::Text("V/S: %.0f fpm", aircraft.getVerticalSpeed() * 196.85);
    ImGui::EndChild();
    
    ImGui::End();
}

void Renderer::drawHorizon(double roll, double pitch) {
    ImGui::SetCursorPos(ImVec2(10, 10));
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    ImVec2 windowPos = ImGui::GetCursorScreenPos();
    ImVec2 center(windowPos.x + windowSize.x * 0.5f, 
                  windowPos.y + windowSize.y * 0.5f);
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    // Simple horizon line
    float pitchOffset = pitch * (180.0 / M_PI) * 3.0f;
    float rollRad = -roll;
    
    float lineLen = windowSize.x * 0.6f;
    ImVec2 horizonLeft(center.x - lineLen * std::cos(rollRad),
                      center.y + pitchOffset - lineLen * std::sin(rollRad));
    ImVec2 horizonRight(center.x + lineLen * std::cos(rollRad),
                       center.y + pitchOffset + lineLen * std::sin(rollRad));
    
    drawList->AddLine(horizonLeft, horizonRight, IM_COL32(255, 255, 255, 200), 3.0f);
    
    // Pitch ladder
    for (int deg = -30; deg <= 30; deg += 10) {
        if (deg == 0) continue;
        
        float offset = (pitch * 180.0 / M_PI + deg) * 3.0f;
        float len = (deg % 20 == 0) ? 60.0f : 40.0f;
        
        ImVec2 p1(center.x - len * std::cos(rollRad),
                 center.y + offset - len * std::sin(rollRad));
        ImVec2 p2(center.x + len * std::cos(rollRad),
                 center.y + offset + len * std::sin(rollRad));
        
        drawList->AddLine(p1, p2, IM_COL32(255, 255, 255, 150), 2.0f);
    }
    
    // Aircraft reference
    drawList->AddLine(ImVec2(center.x - 50, center.y),
                     ImVec2(center.x - 15, center.y),
                     IM_COL32(0, 255, 0, 255), 3.0f);
    drawList->AddLine(ImVec2(center.x + 15, center.y),
                     ImVec2(center.x + 50, center.y),
                     IM_COL32(0, 255, 0, 255), 3.0f);
    drawList->AddCircleFilled(center, 5.0f, IM_COL32(0, 255, 0, 255));
}

void Renderer::drawAircraft() {
    // Simple aircraft representation (could be expanded)
}

void Renderer::drawGround(double altitude) {
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    ImVec2 windowPos = ImGui::GetCursorScreenPos();
    
    // Ground indicator at bottom
    if (altitude < 500.0) {
        float groundY = windowPos.y + windowSize.y - (altitude / 500.0) * (windowSize.y * 0.3f);
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(ImVec2(windowPos.x, groundY),
                               ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y),
                               IM_COL32(139, 90, 43, 100));
    }
}

void Renderer::drawCompass(double heading) {
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    ImVec2 windowPos = ImGui::GetCursorScreenPos();
    
    float compassY = windowPos.y + windowSize.y - 40;
    ImVec2 compassCenter(windowPos.x + windowSize.x * 0.5f, compassY);
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    // Compass tape
    for (int deg = 0; deg < 360; deg += 30) {
        float offset = (deg - heading) * 2.0f;
        if (std::abs(offset) < windowSize.x * 0.5f) {
            ImVec2 pos(compassCenter.x + offset, compassY);
            
            const char* label = "";
            if (deg == 0) label = "N";
            else if (deg == 90) label = "E";
            else if (deg == 180) label = "S";
            else if (deg == 270) label = "W";
            else {
                char buf[8];
                snprintf(buf, sizeof(buf), "%d", deg / 10);
                label = buf;
            }
            
            drawList->AddText(ImVec2(pos.x - 5, pos.y - 10), 
                            IM_COL32(255, 255, 255, 255), label);
        }
    }
    
    // Center marker
    ImVec2 tri[3] = {
        ImVec2(compassCenter.x, compassY - 15),
        ImVec2(compassCenter.x - 8, compassY - 25),
        ImVec2(compassCenter.x + 8, compassY - 25)
    };
    drawList->AddTriangleFilled(tri[0], tri[1], tri[2], IM_COL32(255, 255, 0, 255));
}

