# It is in very early pre-alpha. I have barely gotten the simple functions like objects to work.

[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![C++ Version](https://img.shields.io/badge/C%2B%2B-C%2B%2B20-blue)](https://isocpp.org/)
[![OpenGL Version](https://img.shields.io/badge/OpenGL-3.3-orange)](https://www.opengl.org/)

Nionyx is a lightweight 3D engine written in C++. It focuses on being able to load scenes without having any loading screens. 

---
## Getting Started

### Prerequisites

- C++20 compatible compiler
- CMake 3.20+
- OpenGL 3.3 capable GPU

### Installation

Clone and build:

```bash
git clone https://github.com/Desnio/Nionyx.git
cd Nionyx
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
