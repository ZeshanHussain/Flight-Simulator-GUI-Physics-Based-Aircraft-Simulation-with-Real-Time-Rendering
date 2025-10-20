#pragma once
#include "vector3.hpp"

struct AircraftState {
    // Position (NED frame - North, East, Down)
    Vector3 position;          // m
    
    // Velocity (body frame)
    Vector3 velocity;          // m/s
    
    // Angular velocity (body frame)
    Vector3 angularVelocity;   // rad/s
    
    // Orientation (Euler angles)
    double roll;               // rad
    double pitch;              // rad
    double yaw;                // rad
    
    // Control inputs (-1 to 1)
    double elevator;           // Pitch control
    double aileron;            // Roll control
    double rudder;             // Yaw control
    double throttle;           // 0 to 1
};

class Aircraft {
public:
    Aircraft();
    
    const AircraftState& getState() const { return state; }
    AircraftState& getState() { return state; }
    
    // Get derived parameters
    double getAirspeed() const;
    double getAltitude() const;
    double getVerticalSpeed() const;
    double getAngleOfAttack() const;
    double getSideslip() const;
    double getMachNumber() const;
    
    // Physical properties
    double getMass() const { return mass; }
    double getWingArea() const { return wingArea; }
    double getWingSpan() const { return wingSpan; }
    
    // Aerodynamic coefficients (functions of alpha, beta, controls)
    double getCL(double alpha, double elevator) const;
    double getCD(double alpha) const;
    double getCY(double beta, double rudder) const;
    double getCl(double beta, double aileron, double rudder) const;  // Rolling moment
    double getCm(double alpha, double elevator) const;                // Pitching moment
    double getCn(double beta, double aileron, double rudder) const;  // Yawing moment
    
private:
    AircraftState state;
    
    // Physical properties (Cessna 172 approximate)
    double mass;           // kg
    double wingArea;       // m^2
    double wingSpan;       // m
    double chord;          // m
    
    // Inertia tensor (body frame)
    double Ixx, Iyy, Izz;
    double Ixz;
    
    // Engine
    double maxThrust;      // N
    
    friend class FlightDynamics;
};

