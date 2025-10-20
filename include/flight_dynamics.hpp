#pragma once
#include "aircraft.hpp"
#include "atmosphere.hpp"

class FlightDynamics {
public:
    FlightDynamics(Aircraft* aircraft, Atmosphere* atmosphere);
    
    // Update aircraft state (RK4 integration)
    void update(double dt);
    
    // Reset to initial conditions
    void reset();
    
private:
    Aircraft* aircraft;
    Atmosphere* atmosphere;
    
    // Calculate forces and moments
    Vector3 calculateForces(const AircraftState& state);
    Vector3 calculateMoments(const AircraftState& state);
    
    // State derivative for integration
    struct StateDerivative {
        Vector3 positionDot;
        Vector3 velocityDot;
        Vector3 angularVelocityDot;
        Vector3 eulerDot;
    };
    
    StateDerivative computeDerivative(const AircraftState& state);
    
    // RK4 integration helpers
    AircraftState addScaledDerivative(const AircraftState& state, 
                                     const StateDerivative& deriv, double scale);
};

