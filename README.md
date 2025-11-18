# SpaceSim++

<img src="https://i.imgur.com/Gcu3Qnx.png" width="50%" align="center">

![GitHub Tag](https://img.shields.io/github/v/tag/Damako4/SpaceSimPlusPlus?style=for-the-badge)
![GitHub top language](https://img.shields.io/github/languages/top/Damako4/SpaceSimPlusPlus?style=for-the-badge&color=green)
![GitHub Repo stars](https://img.shields.io/github/stars/Damako4/SpaceSimPlusPlus?style=for-the-badge)

An accurate 3D gravitational physics engine in C.

## Features

- 3D Gravitational simulation
- Custom solar systems (WIP)
- Interactive universe
- Cross platform


## Controls

- **O** - Free Camera Mode (WASD to move, mouse to look around)
- **C** - Orbit Camera Mode (LEFT/RIGHT arrows to orbit)
- **V** - Toggle Grid Visibility
- **W/A/S/D** - Move camera (Free Camera Mode)
- **Mouse** - Look around (Free Camera Mode)
- **ESC** - Exit control mode
- **Left Click** - Select planet


## Installation

1. Install the required system dependencies (GLFW, OpenGL drivers).
2. Download the latest release from the Releases page.
3. Run the `SpaceSimPlusPlus` binary.


## Building the Project

1. **Clone the repository**
```bash
git clone https://github.com/Damako4/SpaceSimPlusPlus.git
cd SpaceSimPlusPlus
```

2. **Install CMake**

Install the latest version of CMake [here](https://cmake.org/) if it's not already installed.

3. **Install Dependencies**

You will need to install the following dependencies and CMake will need to be able to locate them (ie. they need to be listed on your PATH):

    GLFW3 | SOIL2 | GLM | OpenGL


4. **Configure and Build with CMake**

In the project root directory run:
```bash
mkdir build
cd build
cmake ..
cmake --build .

```
5. **Run**

From the `build` directory:
```bash
./SpaceSimPlusPlus
```
