# ⚡ RISC-V GUI Instruction Set Simulator (RV32IM)

![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg?style=flat-square&logo=c%2B%2B)
![CMake](https://img.shields.io/badge/CMake-3.20%2B-green.svg?style=flat-square&logo=cmake)
![OpenGL](https://img.shields.io/badge/GUI-Dear%20ImGui-orange.svg?style=flat-square)

An advanced, high-performance **RISC-V Instruction Set Simulator** featuring an integrated interactive graphical debugging dashboard powered by **Dear ImGui** and **OpenGL**. Built from scratch in **Modern C++ (C++20)** to simulate hardware instruction execution, register state changes, memory tracking, and the RV32M hardware multiplication/division extension.

---

## 🚀 Key Features

*   **Interactive GUI Dashboard**: Real-time inspection controls (Step, Continuous Run, Reset), live register tables, and instruction pointer tracking.
*   **Built-in Disassembler**: Converts raw binary machine code words into readable assembly mnemonics on the fly.
*   **RV32IM Architecture Support**: Handles core base integer instructions alongside hardware multiply (`mul`) and division (`div`, `rem`) extension opcodes.
*   **Zero Heavy Framework Bloat**: Utilizes lightweight, GPU-accelerated immediate-mode GUI libraries (`Dear ImGui` + `GLFW`).

---

## 🏗️ Project Architecture

```text
riscv-gui-iss/
├── CMakeLists.txt         # Automated FetchContent setup for ImGui & GLFW
├── include/
│   └── cpu.hpp            # CPU interface & instruction decoding structures
└── src/
    ├── cpu.cpp            # Execution loop, bitmask decoders, and disassembler logic
    └── main.cpp           # GLFW context manager and Dear ImGui layout loops

# 1. Clone or initialize your directory
cd riscv-gui-iss

# 2. Create and enter the build directory
mkdir build && cd build

# 3. Configure the project using CMake
cmake -DCMAKE_BUILD_TYPE=Release ..

# 4. Compile all targets
cmake --build . -j$(nproc)

# 5. Launch the graphical simulator dashboard
./riscv_gui_sim

