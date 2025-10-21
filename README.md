# 6DOF Flight Simulator

A realistic **6 Degrees of Freedom (6DOF) Flight Simulator** with cockpit instruments, built with C++, OpenGL, and ImGui. This simulator features realistic aerodynamic modeling, atmospheric effects, and a complete set of flight instruments.

## Features

[![Watch the demo on YouTube](https://img.youtube.com/vi/VmB1BS0aFl0/maxresdefault.jpg)](https://youtu.be/VmB1BS0aFl0)


### ✈️ Flight Dynamics
- **6DOF Simulation**: Full rigid body dynamics with forces and moments
- **Realistic Aerodynamics**: Lift, drag, side force, and moments based on angle of attack and control surfaces
- **Cessna 172 Model**: Approximate aerodynamic coefficients and physical properties
- **Atmospheric Model**: ISA (International Standard Atmosphere) with altitude-dependent properties
- **RK4 Integration**: Fourth-order Runge-Kutta integration for accurate state propagation

### 🎮 Flight Instruments
- **Airspeed Indicator**: Displays airspeed in knots
- **Altimeter**: Shows altitude in feet with rotating needle
- **Attitude Indicator**: Artificial horizon showing roll and pitch
- **Heading Indicator**: Directional gyro showing compass heading
- **Vertical Speed Indicator (VSI)**: Rate of climb/descent in feet per minute
- **Turn Coordinator**: Displays turn rate and bank angle
- **Control Surface Indicators**: Shows elevator, aileron, and rudder positions
- **Throttle Gauge**: Engine power setting

### 🖥️ Visualization
- **3D View**: Real-time horizon, pitch ladder, and ground reference
- **Instrument Panel**: Authentic-looking circular gauges
- **Telemetry Display**: Position, velocity, angles, and aerodynamic parameters
- **Control Panel**: Simulation status and instructions

### 🎛️ Controls
| Key(s) | Function |
|--------|----------|
| **W** / **S** or **↑** / **↓** | Elevator (Pitch Control) |
| **A** / **D** or **←** / **→** | Aileron (Roll Control) |
| **Q** / **E** | Rudder (Yaw Control) |
| **Z** / **X** or **PgUp** / **PgDn** | Throttle |
| **Space** | Center controls |
| **P** | Pause/Resume simulation |
| **R** | Reset to initial conditions |
| **ESC** | Exit simulator |

## Requirements

### Dependencies
- **C++17** compiler (GCC 7+, Clang 6+, or MSVC 2017+)
- **CMake** 3.15 or higher
- **OpenGL** 3.3 or higher
- **GLFW3** - Window and input management
- **ImGui** - Immediate mode GUI (included as submodule)

### Platform Support
- ✅ Linux (tested on Arch Linux)
- ✅ macOS
- ✅ Windows

## Installation

### 1. Install System Dependencies

#### Arch Linux / Manjaro
```bash
sudo pacman -S cmake gcc glfw-x11 mesa git
```

#### Ubuntu / Debian
```bash
sudo apt update
sudo apt install build-essential cmake libglfw3-dev libgl1-mesa-dev git
```

#### macOS (with Homebrew)
```bash
brew install cmake glfw
```

#### Windows (with vcpkg)
```cmd
vcpkg install glfw3 opengl
```

### 2. Clone Repository and Setup ImGui

```bash
cd areospace
mkdir -p external
cd external

# Clone Dear ImGui
git clone https://github.com/ocornut/imgui.git
cd imgui
git checkout v1.89.9  # Or latest stable version
cd ../..
```

### 3. Build the Project

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
cmake --build . -j$(nproc)

# The executable will be in build/flight_simulator
```

## Usage

### Running the Simulator

```bash
cd build
./flight_simulator
```

### Initial Conditions
The aircraft starts at:
- **Altitude**: 1000 meters (~3280 feet)
- **Airspeed**: 50 m/s (~97 knots)
- **Attitude**: Level flight
- **Throttle**: 50%

### Flying Tips
1. **Maintain Altitude**: Use small elevator inputs to control pitch
2. **Coordinated Turns**: Use aileron for bank and rudder to coordinate
3. **Throttle Management**: Adjust throttle to maintain desired airspeed
4. **Avoid Stalls**: Keep airspeed above ~40 knots
5. **Watch VSI**: Monitor vertical speed to prevent rapid descents

## Architecture

### Project Structure
```
areospace/
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
├── include/                # Header files
│   ├── vector3.hpp         # 3D vector math
│   ├── quaternion.hpp      # Quaternion rotation
│   ├── atmosphere.hpp      # Atmospheric model
│   ├── aircraft.hpp        # Aircraft state and properties
│   ├── flight_dynamics.hpp # 6DOF dynamics engine
│   ├── instruments.hpp     # Cockpit instruments
│   ├── renderer.hpp        # OpenGL rendering
│   └── input_handler.hpp   # Keyboard/joystick input
├── src/                    # Implementation files
│   ├── main.cpp
│   ├── aircraft.cpp
│   ├── atmosphere.cpp
│   ├── flight_dynamics.cpp
│   ├── instruments.cpp
│   ├── renderer.cpp
│   └── input_handler.cpp
└── external/               # Third-party libraries
    └── imgui/              # Dear ImGui (git submodule)
```

### Key Components

#### Aircraft Model (`aircraft.cpp`)
- Cessna 172 aerodynamic coefficients
- Mass: 1043 kg
- Wing Area: 16.2 m²
- Moments of inertia for realistic dynamics

#### Flight Dynamics (`flight_dynamics.cpp`)
- 6DOF equations of motion
- Forces: Lift, drag, thrust, gravity
- Moments: Roll, pitch, yaw
- RK4 integration for smooth simulation

#### Atmosphere (`atmosphere.cpp`)
- ISA standard atmosphere model
- Altitude-dependent density, pressure, temperature
- Speed of sound calculation

#### Instruments (`instruments.cpp`)
- Realistic gauge rendering using ImGui drawing API
- Circular gauges with needles and scales
- Color-coded indicators

## Physics & Equations

### 6 Degrees of Freedom
The simulator models:
- **3 Translational DOF**: Forward/back, left/right, up/down
- **3 Rotational DOF**: Roll, pitch, yaw

### Aerodynamic Coefficients
```
CL = CL0 + CLα·α + CLδe·δe     (Lift)
CD = CD0 + K·CL²                (Drag)
Cm = Cm0 + Cmα·α + Cmδe·δe     (Pitching moment)
```

### Integration
Using 4th-order Runge-Kutta (RK4) for accurate numerical integration of:
- Position (NED frame)
- Velocity (body frame)
- Angular velocity (body frame)
- Euler angles (roll, pitch, yaw)

## Customization

### Modifying Aircraft Parameters
Edit `aircraft.cpp` to change:
- Mass and inertia
- Aerodynamic coefficients
- Wing geometry
- Engine thrust

### Adding New Instruments
1. Add rendering method in `instruments.hpp`
2. Implement in `instruments.cpp`
3. Call from `render()` method

### Control Mapping
Edit `input_handler.cpp` to customize keyboard bindings or add joystick support.

## Troubleshooting

### Build Issues

**CMake can't find GLFW**
```bash
# Install GLFW development libraries
sudo pacman -S glfw-x11  # Arch
sudo apt install libglfw3-dev  # Ubuntu
```

**ImGui source not found**
```bash
# Make sure ImGui is cloned in external/imgui
cd external
git clone https://github.com/ocornut/imgui.git
```

### Runtime Issues

**Window doesn't appear**
- Check OpenGL drivers are installed
- Try updating graphics drivers

**Simulator is too fast/slow**
- The simulation uses fixed timestep (60 Hz)
- Frame rate is independent of simulation rate

**Aircraft crashes immediately**
- Check initial altitude is positive (NED frame: negative z is up)
- Verify atmospheric model returns valid density

## Future Enhancements

Potential additions:
- [ ] Joystick/HOTAS support
- [ ] Multiple aircraft models
- [ ] Weather effects (wind, turbulence)
- [ ] Navigation (waypoints, GPS)
- [ ] Autopilot modes
- [ ] Multiplayer support
- [ ] Scenery and airports
- [ ] Sound effects
- [ ] VR support

## Technical Details

### Coordinate Systems
- **NED (North-East-Down)**: Inertial reference frame
- **Body Frame**: Aircraft-fixed axes (forward, right, down)
- **Wind Frame**: Aligned with velocity vector

### Euler Angles
- **Roll (φ)**: Rotation about x-axis (right wing down is positive)
- **Pitch (θ)**: Rotation about y-axis (nose up is positive)
- **Yaw (ψ)**: Rotation about z-axis (nose right is positive)

### Control Surfaces
- **Elevator**: Controls pitch (±1.0 normalized)
- **Aileron**: Controls roll (±1.0 normalized)
- **Rudder**: Controls yaw (±1.0 normalized)
- **Throttle**: Controls thrust (0.0 to 1.0)

## References

- Stevens, B. L., & Lewis, F. L. (2003). *Aircraft Control and Simulation*
- Nelson, R. C. (1998). *Flight Stability and Automatic Control*
- International Standard Atmosphere (ISA), ISO 2533:1975
- Dear ImGui: https://github.com/ocornut/imgui
- GLFW: https://www.glfw.org/

## License

This project is provided as-is for educational and simulation purposes.

## Contributing

Contributions are welcome! Areas for improvement:
- More realistic aerodynamic models
- Additional aircraft types
- Enhanced visualization
- Performance optimizations
- Documentation improvements

## Author

Created as a demonstration of aerospace simulation and C++ programming.

---

**Happy Flying! ✈️**

