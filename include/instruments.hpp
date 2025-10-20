#pragma once
#include "aircraft.hpp"

class Instruments {
public:
    Instruments();
    
    // Render all cockpit instruments
    void render(const Aircraft& aircraft);
    
private:
    // Individual instrument rendering
    void renderAltimeter(double altitude);
    void renderAirspeedIndicator(double airspeed);
    void renderAttitudeIndicator(double roll, double pitch);
    void renderHeadingIndicator(double heading);
    void renderVerticalSpeedIndicator(double verticalSpeed);
    void renderTurnCoordinator(double rollRate, double yawRate);
    void renderThrottleGauge(double throttle);
    void renderControlSurfaces(double elevator, double aileron, double rudder);
    
    // Helper for drawing circular gauges
    void drawCircularGauge(float centerX, float centerY, float radius, 
                          const char* label, double value, double minVal, 
                          double maxVal, const char* unit);
    
    // Helper for drawing needles
    void drawNeedle(float centerX, float centerY, float length, float angle, 
                   unsigned int color, float thickness = 2.0f);
};

