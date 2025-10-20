#pragma once

class Atmosphere {
public:
    Atmosphere();
    
    // Get atmospheric properties at altitude (meters)
    void getProperties(double altitude, double& density, double& pressure, 
                      double& temperature, double& speedOfSound);
    
private:
    // ISA (International Standard Atmosphere) constants
    static constexpr double SEA_LEVEL_PRESSURE = 101325.0;    // Pa
    static constexpr double SEA_LEVEL_TEMPERATURE = 288.15;   // K
    static constexpr double SEA_LEVEL_DENSITY = 1.225;        // kg/m^3
    static constexpr double TEMPERATURE_LAPSE_RATE = 0.0065;  // K/m
    static constexpr double GAS_CONSTANT = 287.05;            // J/(kg·K)
    static constexpr double GAMMA = 1.4;                      // Specific heat ratio
    static constexpr double GRAVITY = 9.80665;                // m/s^2
};

