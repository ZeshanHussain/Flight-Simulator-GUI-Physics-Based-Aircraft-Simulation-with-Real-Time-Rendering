#include "atmosphere.hpp"
#include <cmath>

Atmosphere::Atmosphere() {}

void Atmosphere::getProperties(double altitude, double& density, double& pressure, 
                              double& temperature, double& speedOfSound) {
    // Limit altitude to troposphere (0-11000m)
    if (altitude < 0.0) altitude = 0.0;
    
    if (altitude <= 11000.0) {
        // Troposphere
        temperature = SEA_LEVEL_TEMPERATURE - TEMPERATURE_LAPSE_RATE * altitude;
        
        double tempRatio = temperature / SEA_LEVEL_TEMPERATURE;
        double exponent = GRAVITY / (TEMPERATURE_LAPSE_RATE * GAS_CONSTANT);
        
        pressure = SEA_LEVEL_PRESSURE * std::pow(tempRatio, exponent);
        density = pressure / (GAS_CONSTANT * temperature);
    } else {
        // Lower stratosphere (isothermal layer)
        temperature = 216.65; // K
        
        double h11 = 11000.0;
        double T11 = SEA_LEVEL_TEMPERATURE - TEMPERATURE_LAPSE_RATE * h11;
        double P11 = SEA_LEVEL_PRESSURE * std::pow(T11 / SEA_LEVEL_TEMPERATURE, 
                                                     GRAVITY / (TEMPERATURE_LAPSE_RATE * GAS_CONSTANT));
        
        pressure = P11 * std::exp(-GRAVITY * (altitude - h11) / (GAS_CONSTANT * temperature));
        density = pressure / (GAS_CONSTANT * temperature);
    }
    
    speedOfSound = std::sqrt(GAMMA * GAS_CONSTANT * temperature);
}

