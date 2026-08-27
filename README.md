# The Fall of Troy — 2D Animated OpenGL Simulation

An interactive 2D OpenGL animation developed in C++ depicting the legendary **Fall of Troy**. The application features four animated narrative scenes, custom drawing primitives, dynamic camera movement, smooth transformations, particle effects, and keyboard navigation.

---

## 🏛 Scene Overview

1. **Scene 1: Outside the Walls of Troy**
   * View of the high fortress walls of Troy and the city gate.
   * The giant wooden Trojan Horse sitting outside the gate after the Greek fleet seemingly retreats.

2. **Scene 2: Bringing the Horse Inside**
   * The Trojan Horse is rolled on wheels through the main city gates into Troy.
   * Features animated gate opening/closing mechanics, horse scaling, and wheel rotation.

3. **Scene 3: The Night Attack**
   * Set under a moonlight night sky.
   * The trapdoors underneath the Trojan Horse swing open, and Greek soldiers lower down on ropes to infiltrate the sleeping city.

4. **Scene 4: The Fall & Burning of Troy**
   * The climax of the story depicting Troy overrun by flames and turmoil.
   * Features dynamic multi-layered fire particle effects and dramatic red-sky atmosphere.

---

## 🎮 Controls & Navigation

| Key | Action |
| :--- | :--- |
| `1` – `4` | Jump directly to Scene 1, 2, 3, or 4 |
| `N` / `n` | Advance to the **Next** scene |
| `P` / `p` | Go back to the **Previous** scene |
| `ESC` | Exit the application |

---

## 🛠 Prerequisites & Dependencies

To build and run this application, ensure you have the following installed:

* **C++ Compiler**: `g++` or `clang++` supporting C++11 standard
* **Build System**: `CMake` (version 3.10 or higher)
* **Graphics Libraries**:
  * OpenGL
  * GLUT / FreeGLUT
  * GLU

### Installing Dependencies (Linux / Ubuntu / Debian)
```bash
sudo apt update
sudo apt install build-essential cmake libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev
```

---

## 🚀 Building & Running

### Option 1: Using CMake (Recommended)

1. Clone or navigate to the project directory:
   ```bash
   cd troy
   ```
2. Create a `build` directory and run CMake:
   ```bash
   mkdir -p build && cd build
   cmake ..
   make
   ```
3. Run the executable:
   ```bash
   ./troy
   ```

### Option 2: Direct Compilation with `g++`

If you prefer compiling directly using `g++`:
```bash
g++ main.cpp -o troy -lGL -lGLU -lglut
./troy
```

---

## 📁 Repository Structure

```
.
├── CMakeLists.txt   # CMake configuration file
├── main.cpp         # Main C++ source containing OpenGL scenes & logic
└── README.md        # Documentation and project guide
```
