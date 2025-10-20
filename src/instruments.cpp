#include "instruments.hpp"
#include "imgui.h"
#include <cmath>
#include <cstdio>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Instruments::Instruments() {}

void Instruments::render(const Aircraft& aircraft) {
    const AircraftState& state = aircraft.getState();
    
    double altitude = aircraft.getAltitude();
    double airspeed = aircraft.getAirspeed();
    double verticalSpeed = aircraft.getVerticalSpeed();
    double heading = state.yaw * 180.0 / M_PI;
    if (heading < 0) heading += 360.0;
    
    ImGui::Begin("Flight Instruments", nullptr, ImGuiWindowFlags_NoCollapse);
    
    ImGui::Text("Primary Flight Instruments");
    ImGui::Separator();
    
    // Create layout for instruments
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    float instrumentSize = std::min(windowSize.x, windowSize.y) * 0.25f;
    
    // Row 1: Airspeed, Attitude, Altimeter
    ImGui::BeginChild("Row1", ImVec2(0, instrumentSize + 40), false);
    
    ImGui::BeginGroup();
    renderAirspeedIndicator(airspeed);
    ImGui::EndGroup();
    
    ImGui::SameLine();
    ImGui::BeginGroup();
    renderAttitudeIndicator(state.roll, state.pitch);
    ImGui::EndGroup();
    
    ImGui::SameLine();
    ImGui::BeginGroup();
    renderAltimeter(altitude);
    ImGui::EndGroup();
    
    ImGui::EndChild();
    
    // Row 2: Heading, Turn Coordinator, VSI
    ImGui::BeginChild("Row2", ImVec2(0, instrumentSize + 40), false);
    
    ImGui::BeginGroup();
    renderHeadingIndicator(heading);
    ImGui::EndGroup();
    
    ImGui::SameLine();
    ImGui::BeginGroup();
    renderTurnCoordinator(state.angularVelocity.x, state.angularVelocity.z);
    ImGui::EndGroup();
    
    ImGui::SameLine();
    ImGui::BeginGroup();
    renderVerticalSpeedIndicator(verticalSpeed);
    ImGui::EndGroup();
    
    ImGui::EndChild();
    
    // Control surfaces and throttle
    ImGui::Separator();
    ImGui::Text("Controls");
    renderThrottleGauge(state.throttle);
    renderControlSurfaces(state.elevator, state.aileron, state.rudder);
    
    // Additional telemetry
    ImGui::Separator();
    ImGui::Text("Telemetry");
    ImGui::Text("Position: N=%.1f, E=%.1f, D=%.1f m", 
                state.position.x, state.position.y, state.position.z);
    ImGui::Text("Velocity: u=%.1f, v=%.1f, w=%.1f m/s", 
                state.velocity.x, state.velocity.y, state.velocity.z);
    ImGui::Text("Angles: Roll=%.1f°, Pitch=%.1f°, Yaw=%.1f°", 
                state.roll * 180.0 / M_PI, 
                state.pitch * 180.0 / M_PI, 
                state.yaw * 180.0 / M_PI);
    ImGui::Text("Alpha=%.1f°, Beta=%.1f°, Mach=%.3f", 
                aircraft.getAngleOfAttack() * 180.0 / M_PI,
                aircraft.getSideslip() * 180.0 / M_PI,
                aircraft.getMachNumber());
    
    ImGui::End();
}

void Instruments::renderAirspeedIndicator(double airspeed) {
    // Convert m/s to knots
    double knots = airspeed * 1.94384;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float radius = 70.0f;
    ImVec2 center(pos.x + radius + 10, pos.y + radius + 20);
    
    // Background
    drawList->AddCircleFilled(center, radius, IM_COL32(30, 30, 30, 255));
    drawList->AddCircle(center, radius, IM_COL32(200, 200, 200, 255), 0, 2.0f);
    
    // Scale markings (0-200 knots)
    for (int i = 0; i <= 200; i += 20) {
        float angle = -120.0f + (240.0f * i / 200.0f);
        float rad = angle * M_PI / 180.0f;
        float innerRadius = (i % 40 == 0) ? radius - 15.0f : radius - 10.0f;
        
        ImVec2 p1(center.x + std::cos(rad) * innerRadius,
                  center.y + std::sin(rad) * innerRadius);
        ImVec2 p2(center.x + std::cos(rad) * radius,
                  center.y + std::sin(rad) * radius);
        
        drawList->AddLine(p1, p2, IM_COL32(200, 200, 200, 255), 2.0f);
        
        if (i % 40 == 0) {
            char label[8];
            snprintf(label, sizeof(label), "%d", i);
            ImVec2 textPos(center.x + std::cos(rad) * (radius - 25.0f) - 10,
                          center.y + std::sin(rad) * (radius - 25.0f) - 7);
            drawList->AddText(textPos, IM_COL32(200, 200, 200, 255), label);
        }
    }
    
    // Needle
    float needleAngle = -120.0f + (240.0f * std::min(knots, 200.0) / 200.0f);
    drawNeedle(center.x, center.y, radius - 10.0f, needleAngle, IM_COL32(255, 255, 255, 255), 3.0f);
    
    // Center
    drawList->AddCircleFilled(center, 5.0f, IM_COL32(255, 255, 255, 255));
    
    // Label
    ImGui::SetCursorScreenPos(ImVec2(center.x - 30, pos.y));
    ImGui::Text("AIRSPEED");
    ImGui::SetCursorScreenPos(ImVec2(center.x - 30, pos.y + radius * 2 + 25));
    ImGui::Text("%.0f kts", knots);
    
    ImGui::Dummy(ImVec2(radius * 2 + 20, radius * 2 + 40));
}

void Instruments::renderAltimeter(double altitude) {
    // Convert meters to feet
    double feet = altitude * 3.28084;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float radius = 70.0f;
    ImVec2 center(pos.x + radius + 10, pos.y + radius + 20);
    
    // Background
    drawList->AddCircleFilled(center, radius, IM_COL32(30, 30, 30, 255));
    drawList->AddCircle(center, radius, IM_COL32(200, 200, 200, 255), 0, 2.0f);
    
    // Scale markings (0-10000 feet)
    for (int i = 0; i <= 10; i++) {
        float angle = -120.0f + (240.0f * i / 10.0f);
        float rad = angle * M_PI / 180.0f;
        float innerRadius = radius - 15.0f;
        
        ImVec2 p1(center.x + std::cos(rad) * innerRadius,
                  center.y + std::sin(rad) * innerRadius);
        ImVec2 p2(center.x + std::cos(rad) * radius,
                  center.y + std::sin(rad) * radius);
        
        drawList->AddLine(p1, p2, IM_COL32(200, 200, 200, 255), 2.0f);
        
        char label[8];
        snprintf(label, sizeof(label), "%d", i);
        ImVec2 textPos(center.x + std::cos(rad) * (radius - 25.0f) - 5,
                      center.y + std::sin(rad) * (radius - 25.0f) - 7);
        drawList->AddText(textPos, IM_COL32(200, 200, 200, 255), label);
    }
    
    // Needle (for 1000s of feet)
    double thousands = std::fmod(feet / 1000.0, 10.0);
    float needleAngle = -120.0f + (240.0f * thousands / 10.0f);
    drawNeedle(center.x, center.y, radius - 10.0f, needleAngle, IM_COL32(255, 255, 255, 255), 3.0f);
    
    // Center
    drawList->AddCircleFilled(center, 5.0f, IM_COL32(255, 255, 255, 255));
    
    // Label
    ImGui::SetCursorScreenPos(ImVec2(center.x - 30, pos.y));
    ImGui::Text("ALTIMETER");
    ImGui::SetCursorScreenPos(ImVec2(center.x - 30, pos.y + radius * 2 + 25));
    ImGui::Text("%.0f ft", feet);
    
    ImGui::Dummy(ImVec2(radius * 2 + 20, radius * 2 + 40));
}

void Instruments::renderAttitudeIndicator(double roll, double pitch) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float radius = 70.0f;
    ImVec2 center(pos.x + radius + 10, pos.y + radius + 20);
    
    // Clip circle
    drawList->PushClipRect(ImVec2(center.x - radius, center.y - radius),
                          ImVec2(center.x + radius, center.y + radius), true);
    
    // Sky and ground
    float pitchPixels = pitch * (180.0 / M_PI) * 2.0f;
    float horizonY = center.y + pitchPixels;
    
    // Rotate sky/ground with roll
    ImVec2 points[4];
    float cr = std::cos(-roll);
    float sr = std::sin(-roll);
    
    // Sky (blue)
    for (int y = -100; y <= 0; y += 10) {
        float y1 = horizonY + y;
        float y2 = horizonY + y + 10;
        
        for (int x = -100; x <= 100; x += 10) {
            ImVec2 p1(center.x + x, y1);
            ImVec2 p2(center.x + x + 10, y1);
            ImVec2 p3(center.x + x + 10, y2);
            ImVec2 p4(center.x + x, y2);
            
            // Rotate around center
            auto rotate = [&](ImVec2& p) {
                float dx = p.x - center.x;
                float dy = p.y - center.y;
                p.x = center.x + dx * cr - dy * sr;
                p.y = center.y + dx * sr + dy * cr;
            };
            
            rotate(p1); rotate(p2); rotate(p3); rotate(p4);
            
            drawList->AddQuadFilled(p1, p2, p3, p4, IM_COL32(100, 150, 255, 255));
        }
    }
    
    // Ground (brown)
    for (int y = 0; y <= 100; y += 10) {
        float y1 = horizonY + y;
        float y2 = horizonY + y + 10;
        
        for (int x = -100; x <= 100; x += 10) {
            ImVec2 p1(center.x + x, y1);
            ImVec2 p2(center.x + x + 10, y1);
            ImVec2 p3(center.x + x + 10, y2);
            ImVec2 p4(center.x + x, y2);
            
            auto rotate = [&](ImVec2& p) {
                float dx = p.x - center.x;
                float dy = p.y - center.y;
                p.x = center.x + dx * cr - dy * sr;
                p.y = center.y + dx * sr + dy * cr;
            };
            
            rotate(p1); rotate(p2); rotate(p3); rotate(p4);
            
            drawList->AddQuadFilled(p1, p2, p3, p4, IM_COL32(139, 90, 43, 255));
        }
    }
    
    drawList->PopClipRect();
    
    // Outer ring
    drawList->AddCircle(center, radius, IM_COL32(200, 200, 200, 255), 0, 2.0f);
    
    // Aircraft symbol (fixed)
    drawList->AddLine(ImVec2(center.x - 30, center.y), 
                     ImVec2(center.x - 10, center.y), 
                     IM_COL32(255, 255, 0, 255), 3.0f);
    drawList->AddLine(ImVec2(center.x + 10, center.y), 
                     ImVec2(center.x + 30, center.y), 
                     IM_COL32(255, 255, 0, 255), 3.0f);
    drawList->AddCircleFilled(center, 3.0f, IM_COL32(255, 255, 0, 255));
    
    // Label
    ImGui::SetCursorScreenPos(ImVec2(center.x - 30, pos.y));
    ImGui::Text("ATTITUDE");
    ImGui::SetCursorScreenPos(ImVec2(center.x - 40, pos.y + radius * 2 + 25));
    ImGui::Text("R:%.0f° P:%.0f°", roll * 180.0 / M_PI, pitch * 180.0 / M_PI);
    
    ImGui::Dummy(ImVec2(radius * 2 + 20, radius * 2 + 40));
}

void Instruments::renderHeadingIndicator(double heading) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float radius = 70.0f;
    ImVec2 center(pos.x + radius + 10, pos.y + radius + 20);
    
    // Background
    drawList->AddCircleFilled(center, radius, IM_COL32(30, 30, 30, 255));
    drawList->AddCircle(center, radius, IM_COL32(200, 200, 200, 255), 0, 2.0f);
    
    // Compass markings
    const char* cardinals[] = {"N", "3", "6", "E", "12", "15", "S", "21", "24", "W", "30", "33"};
    for (int i = 0; i < 12; i++) {
        float angle = -90.0f + (i * 30.0f) - heading;
        float rad = angle * M_PI / 180.0f;
        
        ImVec2 p1(center.x + std::cos(rad) * (radius - 15.0f),
                  center.y + std::sin(rad) * (radius - 15.0f));
        ImVec2 p2(center.x + std::cos(rad) * radius,
                  center.y + std::sin(rad) * radius);
        
        drawList->AddLine(p1, p2, IM_COL32(200, 200, 200, 255), 2.0f);
        
        ImVec2 textPos(center.x + std::cos(rad) * (radius - 30.0f) - 7,
                      center.y + std::sin(rad) * (radius - 30.0f) - 7);
        drawList->AddText(textPos, IM_COL32(200, 200, 200, 255), cardinals[i]);
    }
    
    // Aircraft heading marker (fixed at top)
    ImVec2 tri[3] = {
        ImVec2(center.x, center.y - radius + 10),
        ImVec2(center.x - 8, center.y - radius + 20),
        ImVec2(center.x + 8, center.y - radius + 20)
    };
    drawList->AddTriangleFilled(tri[0], tri[1], tri[2], IM_COL32(255, 255, 0, 255));
    
    // Label
    ImGui::SetCursorScreenPos(ImVec2(center.x - 30, pos.y));
    ImGui::Text("HEADING");
    ImGui::SetCursorScreenPos(ImVec2(center.x - 20, pos.y + radius * 2 + 25));
    ImGui::Text("%.0f°", heading);
    
    ImGui::Dummy(ImVec2(radius * 2 + 20, radius * 2 + 40));
}

void Instruments::renderVerticalSpeedIndicator(double verticalSpeed) {
    // Convert m/s to feet/min
    double fpm = verticalSpeed * 196.85;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float radius = 70.0f;
    ImVec2 center(pos.x + radius + 10, pos.y + radius + 20);
    
    // Background
    drawList->AddCircleFilled(center, radius, IM_COL32(30, 30, 30, 255));
    drawList->AddCircle(center, radius, IM_COL32(200, 200, 200, 255), 0, 2.0f);
    
    // Scale markings (-2000 to +2000 fpm)
    int marks[] = {-20, -10, 0, 10, 20};
    for (int i = 0; i < 5; i++) {
        float angle = -120.0f + (i * 60.0f);
        float rad = angle * M_PI / 180.0f;
        
        ImVec2 p1(center.x + std::cos(rad) * (radius - 15.0f),
                  center.y + std::sin(rad) * (radius - 15.0f));
        ImVec2 p2(center.x + std::cos(rad) * radius,
                  center.y + std::sin(rad) * radius);
        
        drawList->AddLine(p1, p2, IM_COL32(200, 200, 200, 255), 2.0f);
        
        char label[8];
        snprintf(label, sizeof(label), "%d", marks[i]);
        ImVec2 textPos(center.x + std::cos(rad) * (radius - 30.0f) - 10,
                      center.y + std::sin(rad) * (radius - 30.0f) - 7);
        drawList->AddText(textPos, IM_COL32(200, 200, 200, 255), label);
    }
    
    // Needle
    double clampedFpm = std::max(-2000.0, std::min(2000.0, fpm));
    float needleAngle = (clampedFpm / 2000.0) * 120.0f;
    drawNeedle(center.x, center.y, radius - 10.0f, needleAngle, IM_COL32(255, 255, 255, 255), 3.0f);
    
    // Center
    drawList->AddCircleFilled(center, 5.0f, IM_COL32(255, 255, 255, 255));
    
    // Label
    ImGui::SetCursorScreenPos(ImVec2(center.x - 10, pos.y));
    ImGui::Text("VSI");
    ImGui::SetCursorScreenPos(ImVec2(center.x - 30, pos.y + radius * 2 + 25));
    ImGui::Text("%.0f fpm", fpm);
    
    ImGui::Dummy(ImVec2(radius * 2 + 20, radius * 2 + 40));
}

void Instruments::renderTurnCoordinator(double rollRate, double yawRate) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float width = 160.0f;
    float height = 160.0f;
    ImVec2 center(pos.x + width * 0.5f, pos.y + height * 0.5f);
    
    // Background
    drawList->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), 
                           IM_COL32(30, 30, 30, 255));
    drawList->AddRect(pos, ImVec2(pos.x + width, pos.y + height), 
                     IM_COL32(200, 200, 200, 255), 0.0f, 0, 2.0f);
    
    // Aircraft symbol (tilted based on roll rate)
    float bankAngle = rollRate * 20.0f;  // Scale for visualization
    float cr = std::cos(bankAngle);
    float sr = std::sin(bankAngle);
    
    ImVec2 wing1(-40, 0), wing2(40, 0);
    wing1 = ImVec2(center.x + wing1.x * cr - wing1.y * sr, 
                   center.y + wing1.x * sr + wing1.y * cr);
    wing2 = ImVec2(center.x + wing2.x * cr - wing2.y * sr, 
                   center.y + wing2.x * sr + wing2.y * cr);
    
    drawList->AddLine(wing1, wing2, IM_COL32(255, 255, 0, 255), 4.0f);
    drawList->AddCircleFilled(center, 5.0f, IM_COL32(255, 255, 0, 255));
    
    // Label
    ImGui::SetCursorScreenPos(ImVec2(center.x - 40, pos.y + 5));
    ImGui::Text("TURN COORD");
    ImGui::SetCursorScreenPos(ImVec2(center.x - 50, pos.y + height - 20));
    ImGui::Text("Rate: %.2f rad/s", yawRate);
    
    ImGui::Dummy(ImVec2(width, height));
}

void Instruments::renderThrottleGauge(double throttle) {
    ImGui::Text("Throttle: %.0f%%", throttle * 100.0);
    ImGui::ProgressBar(throttle, ImVec2(-1, 0));
}

void Instruments::renderControlSurfaces(double elevator, double aileron, double rudder) {
    ImGui::Columns(3, "controls", false);
    
    ImGui::Text("Elevator");
    ImGui::ProgressBar((elevator + 1.0) * 0.5, ImVec2(-1, 0), "");
    ImGui::Text("%.2f", elevator);
    
    ImGui::NextColumn();
    ImGui::Text("Aileron");
    ImGui::ProgressBar((aileron + 1.0) * 0.5, ImVec2(-1, 0), "");
    ImGui::Text("%.2f", aileron);
    
    ImGui::NextColumn();
    ImGui::Text("Rudder");
    ImGui::ProgressBar((rudder + 1.0) * 0.5, ImVec2(-1, 0), "");
    ImGui::Text("%.2f", rudder);
    
    ImGui::Columns(1);
}

void Instruments::drawNeedle(float centerX, float centerY, float length, 
                            float angle, unsigned int color, float thickness) {
    float rad = angle * M_PI / 180.0f;
    ImVec2 tip(centerX + std::cos(rad) * length,
               centerY + std::sin(rad) * length);
    ImVec2 base(centerX, centerY);
    
    ImGui::GetWindowDrawList()->AddLine(base, tip, color, thickness);
}

