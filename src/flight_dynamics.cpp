#include "flight_dynamics.hpp"
#include <cmath>

FlightDynamics::FlightDynamics(Aircraft* aircraft, Atmosphere* atmosphere)
    : aircraft(aircraft), atmosphere(atmosphere) {}

void FlightDynamics::update(double dt) {
    // RK4 integration
    AircraftState& state = aircraft->getState();
    
    StateDerivative k1 = computeDerivative(state);
    AircraftState state2 = addScaledDerivative(state, k1, dt * 0.5);
    
    StateDerivative k2 = computeDerivative(state2);
    AircraftState state3 = addScaledDerivative(state, k2, dt * 0.5);
    
    StateDerivative k3 = computeDerivative(state3);
    AircraftState state4 = addScaledDerivative(state, k3, dt);
    
    StateDerivative k4 = computeDerivative(state4);
    
    // Combine derivatives
    state.position += (k1.positionDot + k2.positionDot * 2.0 + k3.positionDot * 2.0 + k4.positionDot) * (dt / 6.0);
    state.velocity += (k1.velocityDot + k2.velocityDot * 2.0 + k3.velocityDot * 2.0 + k4.velocityDot) * (dt / 6.0);
    state.angularVelocity += (k1.angularVelocityDot + k2.angularVelocityDot * 2.0 + k3.angularVelocityDot * 2.0 + k4.angularVelocityDot) * (dt / 6.0);
    
    Vector3 eulerDot = (k1.eulerDot + k2.eulerDot * 2.0 + k3.eulerDot * 2.0 + k4.eulerDot) * (dt / 6.0);
    state.roll += eulerDot.x;
    state.pitch += eulerDot.y;
    state.yaw += eulerDot.z;
    
    // Keep angles in range
    while (state.yaw > M_PI) state.yaw -= 2.0 * M_PI;
    while (state.yaw < -M_PI) state.yaw += 2.0 * M_PI;
    
    // Ground collision
    if (state.position.z > 0.0) {
        state.position.z = 0.0;
        state.velocity = Vector3(0, 0, 0);
        state.angularVelocity = Vector3(0, 0, 0);
    }
}

void FlightDynamics::reset() {
    AircraftState& state = aircraft->getState();
    state.position = Vector3(0, 0, -1000);
    state.velocity = Vector3(50, 0, 0);
    state.angularVelocity = Vector3(0, 0, 0);
    state.roll = 0.0;
    state.pitch = 0.0;
    state.yaw = 0.0;
}

Vector3 FlightDynamics::calculateForces(const AircraftState& state) {
    double altitude = -state.position.z;
    double density, pressure, temperature, speedOfSound;
    atmosphere->getProperties(altitude, density, pressure, temperature, speedOfSound);
    
    double airspeed = state.velocity.magnitude();
    if (airspeed < 0.1) airspeed = 0.1;
    
    double q = 0.5 * density * airspeed * airspeed;  // Dynamic pressure
    
    // Aerodynamic angles
    double alpha = aircraft->getAngleOfAttack();
    double beta = aircraft->getSideslip();
    
    // Aerodynamic forces in body frame
    double CL = aircraft->getCL(alpha, state.elevator);
    double CD = aircraft->getCD(alpha);
    double CY = aircraft->getCY(beta, state.rudder);
    
    // Transform from wind to body frame
    double ca = std::cos(alpha);
    double sa = std::sin(alpha);
    
    Vector3 aeroForce;
    aeroForce.x = q * aircraft->getWingArea() * (-CD * ca + CL * sa);
    aeroForce.y = q * aircraft->getWingArea() * CY;
    aeroForce.z = q * aircraft->getWingArea() * (-CD * sa - CL * ca);
    
    // Thrust
    Vector3 thrust(state.throttle * aircraft->maxThrust, 0, 0);
    
    // Gravity in body frame
    double cr = std::cos(state.roll);
    double sr = std::sin(state.roll);
    double cp = std::cos(state.pitch);
    double sp = std::sin(state.pitch);
    
    Vector3 gravity;
    gravity.x = -aircraft->getMass() * 9.81 * sp;
    gravity.y = aircraft->getMass() * 9.81 * sr * cp;
    gravity.z = aircraft->getMass() * 9.81 * cr * cp;
    
    return aeroForce + thrust + gravity;
}

Vector3 FlightDynamics::calculateMoments(const AircraftState& state) {
    double altitude = -state.position.z;
    double density, pressure, temperature, speedOfSound;
    atmosphere->getProperties(altitude, density, pressure, temperature, speedOfSound);
    
    double airspeed = state.velocity.magnitude();
    if (airspeed < 0.1) airspeed = 0.1;
    
    double q = 0.5 * density * airspeed * airspeed;
    
    double alpha = aircraft->getAngleOfAttack();
    double beta = aircraft->getSideslip();
    
    // Moment coefficients
    double Cl = aircraft->getCl(beta, state.aileron, state.rudder);
    double Cm = aircraft->getCm(alpha, state.elevator);
    double Cn = aircraft->getCn(beta, state.aileron, state.rudder);
    
    Vector3 moments;
    moments.x = q * aircraft->getWingArea() * aircraft->getWingSpan() * Cl;  // Roll
    moments.y = q * aircraft->getWingArea() * aircraft->chord * Cm;          // Pitch
    moments.z = q * aircraft->getWingArea() * aircraft->getWingSpan() * Cn;  // Yaw
    
    return moments;
}

FlightDynamics::StateDerivative FlightDynamics::computeDerivative(const AircraftState& state) {
    StateDerivative deriv;
    
    // Store current state temporarily
    AircraftState originalState = aircraft->getState();
    aircraft->getState() = state;
    
    // Position derivative (transform velocity from body to NED frame)
    double cr = std::cos(state.roll);
    double sr = std::sin(state.roll);
    double cp = std::cos(state.pitch);
    double sp = std::sin(state.pitch);
    double cy = std::cos(state.yaw);
    double sy = std::sin(state.yaw);
    
    deriv.positionDot.x = cy * cp * state.velocity.x + 
                         (cy * sp * sr - sy * cr) * state.velocity.y +
                         (cy * sp * cr + sy * sr) * state.velocity.z;
    deriv.positionDot.y = sy * cp * state.velocity.x +
                         (sy * sp * sr + cy * cr) * state.velocity.y +
                         (sy * sp * cr - cy * sr) * state.velocity.z;
    deriv.positionDot.z = -sp * state.velocity.x +
                          cp * sr * state.velocity.y +
                          cp * cr * state.velocity.z;
    
    // Forces
    Vector3 forces = calculateForces(state);
    
    // Velocity derivative
    double p = state.angularVelocity.x;
    double q = state.angularVelocity.y;
    double r = state.angularVelocity.z;
    
    deriv.velocityDot = forces / aircraft->getMass() - state.angularVelocity.cross(state.velocity);
    
    // Moments
    Vector3 moments = calculateMoments(state);
    
    // Angular velocity derivative (Euler's equations)
    double Ixx = aircraft->Ixx;
    double Iyy = aircraft->Iyy;
    double Izz = aircraft->Izz;
    
    deriv.angularVelocityDot.x = (moments.x - (Izz - Iyy) * q * r) / Ixx;
    deriv.angularVelocityDot.y = (moments.y - (Ixx - Izz) * p * r) / Iyy;
    deriv.angularVelocityDot.z = (moments.z - (Iyy - Ixx) * p * q) / Izz;
    
    // Euler angle derivatives
    deriv.eulerDot.x = p + sr * std::tan(state.pitch) * q + cr * std::tan(state.pitch) * r;
    deriv.eulerDot.y = cr * q - sr * r;
    deriv.eulerDot.z = (sr / cp) * q + (cr / cp) * r;
    
    // Restore original state
    aircraft->getState() = originalState;
    
    return deriv;
}

AircraftState FlightDynamics::addScaledDerivative(const AircraftState& state, 
                                                 const StateDerivative& deriv, double scale) {
    AircraftState newState = state;
    newState.position += deriv.positionDot * scale;
    newState.velocity += deriv.velocityDot * scale;
    newState.angularVelocity += deriv.angularVelocityDot * scale;
    newState.roll += deriv.eulerDot.x * scale;
    newState.pitch += deriv.eulerDot.y * scale;
    newState.yaw += deriv.eulerDot.z * scale;
    return newState;
}

