#include "aircraft.hpp"
#include "atmosphere.hpp"
#include <cmath>

Aircraft::Aircraft() {
    // Initialize state
    state.position = Vector3(0, 0, -1000);  // Start at 1000m altitude
    state.velocity = Vector3(50, 0, 0);     // 50 m/s forward
    state.angularVelocity = Vector3(0, 0, 0);
    state.roll = 0.0;
    state.pitch = 0.0;
    state.yaw = 0.0;
    state.elevator = 0.0;
    state.aileron = 0.0;
    state.rudder = 0.0;
    state.throttle = 0.5;
    
    // Cessna 172 approximate properties
    mass = 1043.0;        // kg
    wingArea = 16.2;      // m^2
    wingSpan = 11.0;      // m
    chord = 1.47;         // m
    
    // Moments of inertia (kg·m^2)
    Ixx = 1285.3;
    Iyy = 1824.9;
    Izz = 2666.9;
    Ixz = 0.0;
    
    maxThrust = 2000.0;   // N
}

double Aircraft::getAirspeed() const {
    return state.velocity.magnitude();
}

double Aircraft::getAltitude() const {
    return -state.position.z;  // NED frame, so negative z is up
}

double Aircraft::getVerticalSpeed() const {
    return -state.velocity.z;  // NED frame
}

double Aircraft::getAngleOfAttack() const {
    double u = state.velocity.x;
    double w = state.velocity.z;
    if (u > 0.1) {
        return std::atan2(w, u);
    }
    return 0.0;
}

double Aircraft::getSideslip() const {
    double v = state.velocity.y;
    double airspeed = getAirspeed();
    if (airspeed > 0.1) {
        return std::asin(v / airspeed);
    }
    return 0.0;
}

double Aircraft::getMachNumber() const {
    Atmosphere atm;
    double density, pressure, temperature, speedOfSound;
    atm.getProperties(getAltitude(), density, pressure, temperature, speedOfSound);
    return getAirspeed() / speedOfSound;
}

// Aerodynamic coefficients (simplified models)
double Aircraft::getCL(double alpha, double elevator) const {
    // Lift coefficient: CL = CL0 + CLalpha * alpha + CLde * elevator
    double CL0 = 0.28;
    double CLalpha = 4.58;  // per radian
    double CLde = 0.36;
    
    return CL0 + CLalpha * alpha + CLde * elevator;
}

double Aircraft::getCD(double alpha) const {
    // Drag coefficient: CD = CD0 + CDi (induced drag)
    double CD0 = 0.027;
    double K = 0.045;  // Induced drag factor
    double CL = getCL(alpha, state.elevator);
    
    return CD0 + K * CL * CL;
}

double Aircraft::getCY(double beta, double rudder) const {
    // Side force coefficient
    double CYbeta = -0.393;
    double CYdr = 0.187;
    
    return CYbeta * beta + CYdr * rudder;
}

double Aircraft::getCl(double beta, double aileron, double rudder) const {
    // Rolling moment coefficient
    double Clbeta = -0.074;
    double Clda = 0.178;
    double Cldr = 0.0147;
    double Clp = -0.484;  // Roll damping
    
    double p = state.angularVelocity.x;
    double pHat = p * wingSpan / (2.0 * getAirspeed());
    
    return Clbeta * beta + Clda * aileron + Cldr * rudder + Clp * pHat;
}

double Aircraft::getCm(double alpha, double elevator) const {
    // Pitching moment coefficient
    double Cm0 = 0.04;
    double Cmalpha = -0.613;
    double Cmde = -1.122;
    double Cmq = -12.4;  // Pitch damping
    
    double q = state.angularVelocity.y;
    double qHat = q * chord / (2.0 * getAirspeed());
    
    return Cm0 + Cmalpha * alpha + Cmde * elevator + Cmq * qHat;
}

double Aircraft::getCn(double beta, double aileron, double rudder) const {
    // Yawing moment coefficient
    double Cnbeta = 0.071;
    double Cnda = -0.0504;
    double Cndr = -0.0805;
    double Cnr = -0.125;  // Yaw damping
    
    double r = state.angularVelocity.z;
    double rHat = r * wingSpan / (2.0 * getAirspeed());
    
    return Cnbeta * beta + Cnda * aileron + Cndr * rudder + Cnr * rHat;
}

