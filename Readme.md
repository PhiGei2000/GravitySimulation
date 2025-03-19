# Gravity simulation
## Overview
A simulation using velocity verlet integration of Newton's law of universal gravitation

### Features
- [x] Velocity verlet integration
- [ ] Runge-Kutta integration
- [ ] GUI
- [ ] General relativity

---
## Getting started
### Requirements
 - C++ 23
 - OpenGL 4.5
 - CMake 3.24.0 and above

### Clone and build
**1. Clone the repository**

    git clone https://github.com/PhiGei2000/GravitySimulation.git
    cd ./GravitySimulation

**2. Run CMake to build the application**

    cmake --build ./build

CMake downloads the following packages during the configuration process:
- [glm](https://github.com/g-truc/glm)
- [glfw](https://github.com/glfw/glfw)
- [glew](https://github.com/nigels-com/glew)

**3. Run the simulation**

    ./build/GravitySimulation

## Change the initial state

You can change the masses, initial positions and velocities inside the runSimulation function inside the main.cpp file
